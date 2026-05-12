#include "TimeGridView.h"
#include "Theme.h"
#include "FontLoader.h"
#include "../core/I18n.h"
#include "../core/Preferences.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolTip>
#include <QPainterPath>
#include <algorithm>

namespace timemaster {

TimeGridView::TimeGridView(Mode mode, QWidget *parent)
    : QWidget(parent), m_mode(mode)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_StyledBackground, false);
    m_currentDate = QDate::currentDate();
    m_contentHeight = m_hourHeight * 24;

    connect(&m_minuteTimer, &QTimer::timeout, this, &TimeGridView::onMinuteTick);
    m_minuteTimer.start(60 * 1000);

    // Repaint when language toggles (weekday labels swap)
    connect(&I18n::instance(), &I18n::languageChanged, this, [this]{ update(); });
    // V4.3 #8 — 周起始日变化时整个网格重排
    connect(&Preferences::instance(), &Preferences::weekStartChanged, this, [this]{
        rebuildLayout();
        update();
    });
}

void TimeGridView::setCurrentDate(const QDate &date) {
    m_currentDate = date;
    rebuildLayout();
    update();
}

void TimeGridView::setEvents(const QList<CalendarEvent> &events) {
    m_events = events;
    rebuildLayout();
    update();
}

void TimeGridView::scrollToHour(int hour) {
    int viewportH = height() - m_headerHeight - m_allDayBandHeight;
    int target = hour * m_hourHeight - viewportH / 4;
    target = qMax(0, target);
    int maxScroll = qMax(0, m_contentHeight - viewportH);
    target = qMin(target, maxScroll);
    m_scrollY = target;
    update();
}

QList<QDate> TimeGridView::visibleDays() const {
    // V4.2 #11: Day mode shows 2 days side-by-side.
    if (m_mode == DayMode) return { m_currentDate, m_currentDate.addDays(1) };
    QList<QDate> days;
    // V4.3 #8 — 周起始日由 Preferences 决定（周一 or 周日）
    QDate start = Preferences::instance().weekStartOf(m_currentDate);
    for (int i = 0; i < 7; ++i) days << start.addDays(i);
    return days;
}

void TimeGridView::onMinuteTick() { update(); }

void TimeGridView::rebuildLayout() {
    m_eventRects.clear();
    if (width() <= 0 || height() <= 0) return;

    auto days = visibleDays();
    int nDays = days.size();

    int allDayMaxRows = 0;
    for (const auto &d : days) {
        int n = 0;
        for (const auto &e : m_events) {
            if (e.allDay && e.startDate.date() <= d && e.endDate.date() >= d) n++;
        }
        allDayMaxRows = qMax(allDayMaxRows, n);
    }
    allDayMaxRows = qMin(allDayMaxRows, 3);
    m_allDayBandHeight = allDayMaxRows > 0 ? (allDayMaxRows * 22 + 8) : 0;

    int gridLeft = m_timeGutter;
    int gridTop = m_headerHeight + m_allDayBandHeight;
    double colW = double(width() - gridLeft) / nDays;

    for (int di = 0; di < nDays; ++di) {
        const auto d = days[di];
        QList<CalendarEvent> allDay;
        for (const auto &e : m_events) {
            if (e.allDay && e.startDate.date() <= d && e.endDate.date() >= d)
                allDay.append(e);
        }
        for (int j = 0; j < qMin(3, allDay.size()); ++j) {
            QRect r(int(gridLeft + di * colW + 2),
                    m_headerHeight + 4 + j * 22,
                    int(colW - 4),
                    20);
            m_eventRects.append({r, allDay[j]});
        }
    }

    for (int di = 0; di < nDays; ++di) {
        QRect dayCol(int(gridLeft + di * colW), gridTop,
                     int(colW), m_contentHeight);
        m_eventRects.append(layoutDayEvents(days[di], dayCol));
    }
}

QList<TimeGridView::EventLayout> TimeGridView::layoutDayEvents(const QDate &date, const QRect &dayCol) {
    QList<EventLayout> out;
    QList<CalendarEvent> dayEvents;

    QDateTime dayStart(date, QTime(0, 0, 0));
    QDateTime dayEnd = dayStart.addDays(1);

    for (const auto &e : m_events) {
        if (e.allDay) continue;
        if (e.endDate <= dayStart) continue;
        if (e.startDate >= dayEnd) continue;
        dayEvents.append(e);
    }

    std::sort(dayEvents.begin(), dayEvents.end(),
              [](const CalendarEvent &a, const CalendarEvent &b) {
                  if (a.startDate != b.startDate) return a.startDate < b.startDate;
                  return a.endDate > b.endDate;
              });

    QList<QList<int>> columns;
    QList<int> assigned(dayEvents.size(), -1);

    for (int i = 0; i < dayEvents.size(); ++i) {
        const auto &cur = dayEvents[i];
        bool placed = false;
        for (int c = 0; c < columns.size(); ++c) {
            int last = columns[c].last();
            if (dayEvents[last].endDate <= cur.startDate) {
                columns[c].append(i);
                assigned[i] = c;
                placed = true;
                break;
            }
        }
        if (!placed) {
            columns.append({i});
            assigned[i] = columns.size() - 1;
        }
    }

    auto overlapping = [&](int idx) {
        QList<int> ov;
        ov.append(idx);
        for (int j = 0; j < dayEvents.size(); ++j) {
            if (j == idx) continue;
            if (dayEvents[j].startDate < dayEvents[idx].endDate &&
                dayEvents[j].endDate > dayEvents[idx].startDate) {
                ov.append(j);
            }
        }
        return ov;
    };

    for (int i = 0; i < dayEvents.size(); ++i) {
        const auto &e = dayEvents[i];
        QList<int> ov = overlapping(i);
        int maxCol = 0;
        for (int j : ov) maxCol = qMax(maxCol, assigned[j]);
        int totalCols = maxCol + 1;
        int col = assigned[i];

        int dayMins = 24 * 60;
        QDateTime startInDay = qMax(e.startDate, QDateTime(date, QTime(0, 0, 0)));
        QDateTime endInDay = qMin(e.endDate, QDateTime(date.addDays(1), QTime(0, 0, 0)));
        double startM = startInDay.time().hour() * 60.0 + startInDay.time().minute();
        if (startInDay.date() != date) startM = 0;
        double endM = endInDay.time().hour() * 60.0 + endInDay.time().minute();
        if (endInDay.date() != date && endInDay > startInDay) endM = dayMins;
        if (endM <= startM) endM = startM + 30;

        int top = int(startM / 60.0 * m_hourHeight);
        int hgt = int((endM - startM) / 60.0 * m_hourHeight);
        // V4.3 #6 / V4.4 #3 — 短日程显示最小高度。V4.3 设为 36，但 #3 把
        // 标题区域顶部 padding 从 4 提到 7，36 已经放不下"标题+时间"两行，
        // 这里再提到 44：7 (top) + 22 (title) + 14 (time) + 1 (bottom) = 44。
        if (hgt < 44) hgt = 44;

        double colW = double(dayCol.width() - 4) / totalCols;
        int x = dayCol.left() + 2 + int(col * colW);
        int w = qMax(20, int(colW - 2));

        QRect r(x, dayCol.top() + top, w, hgt);
        out.append({r, e});
    }
    return out;
}

void TimeGridView::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    auto &theme = Theme::instance();
    auto pal = theme.palette();

    p.fillRect(rect(), Qt::transparent);

    auto days = visibleDays();
    int nDays = days.size();
    int gridLeft = m_timeGutter;
    int gridTop = m_headerHeight + m_allDayBandHeight;
    double colW = double(width() - gridLeft) / nDays;

    QDate today = QDate::currentDate();

    // 头部背景（半透明，让卡片背景透出）
    QRect headRect(0, 0, width(), m_headerHeight);
    QColor headBg = theme.bgContainer();
    headBg.setAlphaF(0.5);
    p.fillRect(headRect, headBg);

    bool en = I18n::instance().isEnglish();
    static const char *weekdaysZh[] = {"周日","周一","周二","周三","周四","周五","周六"};
    static const char *weekdaysEn[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    const char *const *weekdays = en ? weekdaysEn : weekdaysZh;
    for (int i = 0; i < nDays; ++i) {
        QRect col(int(gridLeft + i * colW), 0, int(colW), m_headerHeight);
        QDate d = days[i];
        bool isToday = (d == today);

        // V4.2 #1 — weekday label (Mon/Tue/...) bold, never overlapped by the
        // today highlight. Top zone reserved 8..26 px.
        QFont f1 = font();
        f1.setPointSize(11);
        f1.setWeight(QFont::Bold);
        p.setFont(f1);
        int dowIdx = d.dayOfWeek() % 7;  // Sun=0..Sat=6
        if (isToday) {
            p.setPen(theme.brand());
        } else {
            p.setPen((dowIdx == 0 || dowIdx == 6) ? theme.danger() : theme.textSecondary());
        }
        QRect topR(col.left(), 8, col.width(), 18);
        p.drawText(topR, Qt::AlignCenter, weekdays[dowIdx]);

        // V4.2 #1 — today highlight: smaller circle (28px) placed BELOW the
        // weekday text with a 6px gap, never covering the text above. The
        // brand-color column tint (set further down) is also softened.
        if (isToday) {
            // V4.1 #7 — today numeric matches sibling day fontsize (15pt DemiBold,
            // V4.2 — slightly smaller so the circle has comfortable breathing room).
            QFont fnum = font();
            fnum.setFamily(FontLoader::numericFamily());
            fnum.setPointSize(15);
            fnum.setWeight(QFont::DemiBold);
            p.setFont(fnum);

            int diam = 28;  // V4.2: shrunk from 40 -> 28
            int circleTop = 32;  // V4.2: 6px gap below weekday label
            QRect circle(col.center().x() - diam / 2, circleTop, diam, diam);
            p.setBrush(theme.brand());
            p.setPen(Qt::NoPen);
            p.drawEllipse(circle);
            p.setPen(Qt::white);
            p.drawText(circle, Qt::AlignCenter, QString::number(d.day()));
        } else {
            QFont f2 = font();
            f2.setFamily(FontLoader::numericFamily());
            f2.setPointSize(15);
            f2.setWeight(QFont::DemiBold);
            p.setFont(f2);
            p.setPen((dowIdx == 0 || dowIdx == 6) ? theme.danger() : theme.textPrimary());
            QRect dR(col.left(), 32, col.width(), 28);
            p.drawText(dR, Qt::AlignCenter, QString::number(d.day()));
        }
    }

    p.setPen(theme.stroke());
    p.drawLine(0, m_headerHeight, width(), m_headerHeight);

    if (m_allDayBandHeight > 0) {
        QRect bandR(0, m_headerHeight, width(), m_allDayBandHeight);
        QColor bandC = theme.bgContainer();
        bandC.setAlphaF(0.3);
        p.fillRect(bandR, bandC);
        p.drawLine(0, m_headerHeight + m_allDayBandHeight,
                   width(), m_headerHeight + m_allDayBandHeight);

        QFont sf = font(); sf.setPointSize(9);
        p.setFont(sf);
        p.setPen(theme.textPlaceholder());
        p.drawText(QRect(8, m_headerHeight, m_timeGutter - 8, 22),
                   Qt::AlignLeft | Qt::AlignVCenter, I18n::t("event.all_day"));
    }

    p.save();
    QRect gridArea(0, gridTop, width(), height() - gridTop);
    p.setClipRect(gridArea);

    QFont hf = font();
    hf.setFamily(FontLoader::numericFamily());
    hf.setPointSize(9);
    p.setFont(hf);
    for (int h = 1; h < 24; ++h) {
        int y = gridTop + h * m_hourHeight - m_scrollY;
        if (y < gridTop || y > height()) continue;

        p.setPen(QPen(theme.stroke(), 1));
        p.drawLine(m_timeGutter, y, width(), y);

        p.setPen(theme.textPlaceholder());
        QString label = QString("%1:00").arg(h, 2, 10, QChar('0'));
        p.drawText(QRect(0, y - 8, m_timeGutter - 8, 16),
                   Qt::AlignRight | Qt::AlignVCenter, label);
    }

    p.setPen(QPen(theme.stroke(), 1));
    for (int i = 0; i <= nDays; ++i) {
        int x = int(gridLeft + i * colW);
        p.drawLine(x, gridTop - m_allDayBandHeight, x, height());
    }

    // V4.2 #1 — today column highlight much softer (alpha 0.08 vs old 0.3).
    // Applies in both Week mode and the new 2-day Day mode.
    for (int i = 0; i < nDays; ++i) {
        if (days[i] == today) {
            QRect r(int(gridLeft + i * colW), gridTop,
                    int(colW), height() - gridTop);
            QColor c = theme.brand();
            c.setAlphaF(0.08);
            p.fillRect(r, c);
        }
    }

    p.restore();

    for (const auto &er : m_eventRects) {
        QRect r = er.rect;
        if (!er.event.allDay) {
            r.translate(0, -m_scrollY);
            if (r.bottom() < gridTop || r.top() > height()) continue;
            if (r.top() < gridTop) r.setTop(gridTop);
        }
        auto &c = pal[er.event.color];

        // Apple Calendar 风：四角统一 6px 圆角；左侧 3px 内嵌彩条同样圆角
        QPainterPath path;
        path.addRoundedRect(r, 6, 6);
        p.fillPath(path, c.bg);

        QRect bar(r.left() + 2, r.top() + 3, 3, qMax(0, r.height() - 6));
        QPainterPath barPath;
        barPath.addRoundedRect(bar, 1.5, 1.5);
        p.fillPath(barPath, c.text);

        // V4.4 #3 — 标题文本框顶部 padding 4 → 7，给中文字符的顶部笔画
        // （比如"接"的提手旁、"深"的氵）留出呼吸空间。配套地标题高度
        // 18 → 22，避免 descender 被裁。
        QRect tr(r.left() + 10, r.top() + 7, r.width() - 14, r.height() - 10);
        p.setPen(c.text);

        QFont f = font(); f.setPointSize(11); f.setWeight(QFont::DemiBold);
        p.setFont(f);
        QFontMetrics fm(f);
        QString title = fm.elidedText(er.event.title, Qt::ElideRight, tr.width());
        p.drawText(QRect(tr.left(), tr.top(), tr.width(), 22),
                   Qt::AlignLeft | Qt::AlignTop, title);

        if (!er.event.allDay) {
            // V4.4 #3 — 标题用 22px 后，第二行（时间）起点上移 4px
            int yCursor = tr.top() + 22;
            const int kRowH = 14;

            if (r.height() > 32) {
                QFont ft = font();
                ft.setFamily(FontLoader::numericFamily());
                ft.setPointSize(9);
                p.setFont(ft);
                QString ts = er.event.startDate.toString("HH:mm")
                           + " - " + er.event.endDate.toString("HH:mm");
                p.drawText(QRect(tr.left(), yCursor, tr.width(), 16),
                           Qt::AlignLeft | Qt::AlignTop, ts);
                yCursor += kRowH;
            }

            if (!er.event.location.isEmpty() && (yCursor + kRowH) <= tr.bottom()) {
                QFont fl = font(); fl.setPointSize(9);
                p.setFont(fl);
                QFontMetrics flm(fl);
                QString loc = "📍 " + er.event.location;
                loc = flm.elidedText(loc, Qt::ElideRight, tr.width());
                p.drawText(QRect(tr.left(), yCursor, tr.width(), 16),
                           Qt::AlignLeft | Qt::AlignTop, loc);
                yCursor += kRowH;
            }

            if (!er.event.description.isEmpty() && (yCursor + kRowH) <= tr.bottom()) {
                QFont fd = font(); fd.setPointSize(9); fd.setItalic(true);
                p.setFont(fd);
                QFontMetrics fdm(fd);
                QString desc = er.event.description;
                int nl = desc.indexOf('\n');
                if (nl >= 0) desc = desc.left(nl);
                desc = fdm.elidedText(desc, Qt::ElideRight, tr.width());
                QColor descColor = c.text;
                descColor.setAlphaF(0.78);
                p.setPen(descColor);
                p.drawText(QRect(tr.left(), yCursor, tr.width(), 16),
                           Qt::AlignLeft | Qt::AlignTop, desc);
            }
        }
    }

    QDate now = QDate::currentDate();
    QTime nt = QTime::currentTime();
    bool drawNow = false;
    int nowCol = -1;
    for (int i = 0; i < nDays; ++i) {
        if (days[i] == now) { drawNow = true; nowCol = i; break; }
    }
    if (drawNow) {
        int mins = nt.hour() * 60 + nt.minute();
        int y = gridTop + int(mins / 60.0 * m_hourHeight) - m_scrollY;
        if (y > gridTop && y < height()) {
            int xL = int(gridLeft + nowCol * colW);
            int xR = int(gridLeft + (nowCol + 1) * colW);
            p.setBrush(theme.nowLine());
            p.setPen(Qt::NoPen);
            p.drawEllipse(QPoint(xL, y), 4, 4);
            p.setPen(QPen(theme.nowLine(), 2));
            p.drawLine(xL + 4, y, xR, y);
        }
    }
}

void TimeGridView::resizeEvent(QResizeEvent *) {
    rebuildLayout();
    update();
}

void TimeGridView::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton) return;
    for (const auto &er : m_eventRects) {
        QRect r = er.rect;
        if (!er.event.allDay) r.translate(0, -m_scrollY);
        if (r.contains(e->pos())) {
            emit eventClicked(er.event);
            return;
        }
    }
}

void TimeGridView::mouseDoubleClickEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton) return;
    QDateTime dt = hitTimeSlot(e->pos());
    if (dt.isValid()) emit timeSlotClicked(dt);
}

QDateTime TimeGridView::hitTimeSlot(const QPoint &pos) const {
    int gridLeft = m_timeGutter;
    int gridTop = m_headerHeight + m_allDayBandHeight;
    if (pos.x() < gridLeft || pos.y() < gridTop) return QDateTime();

    auto days = visibleDays();
    int nDays = days.size();
    double colW = double(width() - gridLeft) / nDays;
    int colIdx = qBound(0, int((pos.x() - gridLeft) / colW), nDays - 1);

    int yInGrid = pos.y() - gridTop + m_scrollY;
    int hour = qBound(0, yInGrid / m_hourHeight, 23);
    return QDateTime(days[colIdx], QTime(hour, 0, 0));
}

void TimeGridView::mouseMoveEvent(QMouseEvent *e) {
    for (const auto &er : m_eventRects) {
        QRect r = er.rect;
        if (!er.event.allDay) r.translate(0, -m_scrollY);
        if (r.contains(e->pos())) {
            QString tip = er.event.title;
            if (!er.event.allDay) {
                tip += "\n" + er.event.startDate.toString("HH:mm")
                    + " - " + er.event.endDate.toString("HH:mm");
            }
            if (!er.event.location.isEmpty()) {
                tip += "\n📍 " + er.event.location;
            }
            if (!er.event.description.isEmpty()) {
                tip += "\n" + er.event.description;
            }
            QToolTip::showText(e->globalPosition().toPoint(), tip, this);
            return;
        }
    }
    QToolTip::hideText();
}

void TimeGridView::wheelEvent(QWheelEvent *e) {
    int dy = e->angleDelta().y();

    if (e->modifiers() & Qt::ControlModifier) {
        if (dy == 0) { e->accept(); return; }

        constexpr double kStepPerUnit = 4.0 / 120.0;
        double targetH = m_hourHeight + dy * kStepPerUnit;

        constexpr int kMinH = 56;
        constexpr int kMaxH = int(56 * 3.14 + 0.5);
        int newH = qBound(double(kMinH), targetH, double(kMaxH));
        if (newH == m_hourHeight) { e->accept(); return; }

        int gridTop = m_headerHeight + m_allDayBandHeight;
        double anchorTimeY = qMax(0, e->position().toPoint().y() - gridTop) + m_scrollY;
        double anchorRatio = (m_hourHeight > 0) ? anchorTimeY / double(m_hourHeight) : 0;

        m_hourHeight = newH;
        m_contentHeight = m_hourHeight * 24;
        m_scrollY = qMax(0, int(anchorRatio * m_hourHeight - (e->position().toPoint().y() - gridTop)));
        int viewportH = height() - m_headerHeight - m_allDayBandHeight;
        int maxScroll = qMax(0, m_contentHeight - viewportH);
        m_scrollY = qMin(m_scrollY, maxScroll);

        rebuildLayout();
        update();
        e->accept();
        return;
    }

    int viewportH = height() - m_headerHeight - m_allDayBandHeight;
    int maxScroll = qMax(0, m_contentHeight - viewportH);
    m_scrollY = qBound(0, m_scrollY - dy, maxScroll);
    update();
    e->accept();
}

void TimeGridView::leaveEvent(QEvent *) { QToolTip::hideText(); }

} // namespace timemaster
