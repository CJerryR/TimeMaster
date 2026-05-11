#include "TimeGridView.h"
#include "Theme.h"

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
    if (m_mode == DayMode) return { m_currentDate };
    QList<QDate> days;
    int dow = m_currentDate.dayOfWeek() % 7;
    QDate start = m_currentDate.addDays(-dow);
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
        if (hgt < 18) hgt = 18;

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

    static const char *weekdays[] = {"周日","周一","周二","周三","周四","周五","周六"};
    for (int i = 0; i < nDays; ++i) {
        QRect col(int(gridLeft + i * colW), 0, int(colW), m_headerHeight);
        QDate d = days[i];
        bool isToday = (d == today);

        // 周几标签：加粗
        QFont f1 = font();
        f1.setPointSize(10);
        f1.setWeight(QFont::Bold);
        p.setFont(f1);
        p.setPen(isToday ? theme.brand() : theme.textSecondary());
        QRect topR(col.left(), 8, col.width(), 16);
        p.drawText(topR, Qt::AlignCenter, weekdays[d.dayOfWeek() % 7]);

        if (isToday) {
            // 高亮圆与数字之间留出 4px 内边距，数字稍小一些保持加粗
            QFont fnum = font();
            fnum.setPointSize(16);
            fnum.setWeight(QFont::Bold);
            p.setFont(fnum);

            int diam = 34;  // 比之前(32)大一圈，把数字裹得更宽松
            QRect circle(col.center().x() - diam / 2, 24, diam, diam);
            p.setBrush(theme.brand());
            p.setPen(Qt::NoPen);
            p.drawEllipse(circle);
            p.setPen(Qt::white);
            p.drawText(circle, Qt::AlignCenter, QString::number(d.day()));
        } else {
            QFont f2 = font();
            f2.setPointSize(19);
            f2.setWeight(QFont::DemiBold);
            p.setFont(f2);
            int dow = d.dayOfWeek() % 7;
            p.setPen((dow == 0 || dow == 6) ? theme.danger() : theme.textPrimary());
            QRect dR(col.left(), 26, col.width(), 30);
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
                   Qt::AlignLeft | Qt::AlignVCenter, "全天");
    }

    p.save();
    QRect gridArea(0, gridTop, width(), height() - gridTop);
    p.setClipRect(gridArea);

    QFont hf = font(); hf.setPointSize(9);
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

    if (m_mode == WeekMode) {
        for (int i = 0; i < nDays; ++i) {
            if (days[i] == today) {
                QRect r(int(gridLeft + i * colW), gridTop,
                        int(colW), height() - gridTop);
                QColor c = theme.todayHighlight();
                c.setAlphaF(0.3);
                p.fillRect(r, c);
            }
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

        // Apple Calendar 风：四角统一 6px 圆角；左侧 3px 内嵌彩条同样圆角，
        // 而不是把整个块切掉。
        QPainterPath path;
        path.addRoundedRect(r, 6, 6);
        p.fillPath(path, c.bg);

        // 左侧 strip：内缩 1px，单独圆角，不再硬切
        QRect bar(r.left() + 2, r.top() + 3, 3, qMax(0, r.height() - 6));
        QPainterPath barPath;
        barPath.addRoundedRect(bar, 1.5, 1.5);
        p.fillPath(barPath, c.text);

        QRect tr(r.left() + 10, r.top() + 4, r.width() - 14, r.height() - 6);
        p.setPen(c.text);

        QFont f = font(); f.setPointSize(11); f.setWeight(QFont::DemiBold);
        p.setFont(f);
        QFontMetrics fm(f);
        QString title = fm.elidedText(er.event.title, Qt::ElideRight, tr.width());
        p.drawText(QRect(tr.left(), tr.top(), tr.width(), 18),
                   Qt::AlignLeft | Qt::AlignTop, title);

        if (r.height() > 28 && !er.event.allDay) {
            QFont f2 = font(); f2.setPointSize(9);
            p.setFont(f2);
            QString ts = er.event.startDate.toString("HH:mm")
                       + " - " + er.event.endDate.toString("HH:mm");
            p.drawText(QRect(tr.left(), tr.top() + 18, tr.width(), 16),
                       Qt::AlignLeft | Qt::AlignTop, ts);
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
            int xL = (m_mode == DayMode) ? gridLeft : int(gridLeft + nowCol * colW);
            int xR = (m_mode == DayMode) ? width() : int(gridLeft + (nowCol + 1) * colW);
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
            QToolTip::showText(e->globalPosition().toPoint(), tip, this);
            return;
        }
    }
    QToolTip::hideText();
}

void TimeGridView::wheelEvent(QWheelEvent *e) {
    int dy = e->angleDelta().y();
    int viewportH = height() - m_headerHeight - m_allDayBandHeight;
    int maxScroll = qMax(0, m_contentHeight - viewportH);
    m_scrollY = qBound(0, m_scrollY - dy, maxScroll);
    update();
    e->accept();
}

void TimeGridView::leaveEvent(QEvent *) { QToolTip::hideText(); }

} // namespace timemaster
