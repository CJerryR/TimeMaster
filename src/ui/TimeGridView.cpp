//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

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

// 构造函数：初始化分钟定时器，连接语言和周起始日变化信号
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

// 设置当前日期并触发重建和重绘
void TimeGridView::setCurrentDate(const QDate &date) {
    m_currentDate = date;
    rebuildLayout();
    update();
}

// 设置事件列表并触发重建和重绘
void TimeGridView::setEvents(const QList<CalendarEvent> &events) {
    m_events = events;
    rebuildLayout();
    update();
}

// 滚动视图到指定小时位置（使得该小时位于视口上方四分之一处）
void TimeGridView::scrollToHour(int hour) {
    int viewportH = height() - m_headerHeight - m_allDayBandHeight;
    int target = hour * m_hourHeight - viewportH / 4;
    target = qMax(0, target);
    int maxScroll = qMax(0, m_contentHeight - viewportH);
    target = qMin(target, maxScroll);
    m_scrollY = target;
    update();
}

// 返回当前视图可见日期列表：日模式2天，周模式7天
QList<QDate> TimeGridView::visibleDays() const {
    // V4.2 #11: Day mode shows 2 days side-by-side.
    if (m_mode == DayMode) return { m_currentDate, m_currentDate.addDays(1) };
    QList<QDate> days;
    // V4.3 #8 — 周起始日由 Preferences 决定（周一 or 周日）
    QDate start = Preferences::instance().weekStartOf(m_currentDate);
    for (int i = 0; i < 7; ++i) days << start.addDays(i);
    return days;
}

// 每分钟触发：更新时间红线
void TimeGridView::onMinuteTick() { update(); }

// 重建布局：计算全天事件带高度，分配所有事件矩形位置
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

// 对单日事件进行列分配和坐标计算：排序、冲突检测、列宽均分
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
        // V4.3.2 #3 — 文字起点上移 14px 后，最小高度配套调整 44 → 30：
        // 0 padding + 18 (title) + 14 (time row) ≈ 32，留 -2 容差，30 视觉上足够。
        if (hgt < 30) hgt = 30;

        double colW = double(dayCol.width() - 4) / totalCols;
        int x = dayCol.left() + 2 + int(col * colW);
        int w = qMax(20, int(colW - 2));

        QRect r(x, dayCol.top() + top, w, hgt);
        out.append({r, e});
    }
    return out;
}

// 绘制时间网格：日期头部、时间轴、背景网格、全天事件带、事件色块、当前时间红线
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
            int circleTop = 37;  // V4.3.2 #2 — 整体下移 5px (32 → 37) 让红圈
                                 // 不再贴近上方"周X"文字，呼吸感更舒服
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
            QRect dR(col.left(), 37, col.width(), 28);  // V4.3.2 #2 — 同步红圈 +5
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

        // V4.3.2 #3 — 日程块"文字起始位置向上 14px"：撤销 V4.4 #3 加的 padding，
        // 文字回到贴近色块顶部的视觉位置。配套：
        //   · 标题高度 22 → 18（V4.3 原值），AlignTop 让 cap-line 几乎贴 r.top()，
        //     Qt 的 AlignTop 语义不会让 ascender 越出 rect 顶边，所以不会触发
        //     V4.4 #3 修的中文笔画裁切。
        //   · 第二行起点 22 → 14，配合标题区缩回；视觉上时间行整体上移约 15px。
        //   · 最小事件高度同步：44 → 30（见 layoutDayEvents 里的最小值 clamp）。
        // V4.3.3 #3 — 实测短日程的描述行（最后一行）还会被色块底缘 / 圆角遮一部分。
        // 在 V4.3.2 #3 基础上再把整段文字（含标题 + 时间 + 地点 + 描述）整体上移
        // 3px。AlignTop 把标题 cap-line 上移到 r.top()-3，相对 r.top() 的 18px
        // 标题矩形高度不变（也就是矩形底也跟着上移 3px），所以标题底部 CJK 笔画
        // 距矩形底的余量与上版相同，没有引入新的顶部裁切；同时下方所有行连带
        // 上移 3px，给"下面"留出 3px 余量。
        QRect tr(r.left() + 10, r.top(), r.width() - 14, r.height() - 2);
        p.setPen(c.text);

        QFont f = font(); f.setPointSize(11); f.setWeight(QFont::DemiBold);
        p.setFont(f);
        QFontMetrics fm(f);
        QString title = fm.elidedText(er.event.title, Qt::ElideRight, tr.width());
        // V4.3.3 #3 — 标题 y 由 tr.top() → tr.top() - 3。仅对时间网格里的
        // 多行时段事件生效；全天事件（all-day band，色块只有 20px 高、只有
        // 一行标题）没有"下面被遮挡"的问题，再上移反而会让 cap 浮在色块
        // 上方，所以保持原位。
        int titleYAdj = er.event.allDay ? 0 : -3;
        p.drawText(QRect(tr.left(), tr.top() + titleYAdj, tr.width(), 18),
                   Qt::AlignLeft | Qt::AlignTop, title);

        if (!er.event.allDay) {
            // V4.3.2 #3 — 第二行起点从 +22 改到 +14，整体上移
            // V4.3.3 #3 — 在 V4.3.2 基础上再 -3，下游行（地点 / 描述）通过
            // kRowH 累加，会自然跟着上移 3px。
            int yCursor = tr.top() + 11;
            const int kRowH = 14;

            if (r.height() > 24) {
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

// 尺寸变化时重建布局并重绘
void TimeGridView::resizeEvent(QResizeEvent *) {
    rebuildLayout();
    update();
}

// 鼠标按下：检测事件点击
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

// 鼠标双击：检测时间槽点击并触发新建事件信号
void TimeGridView::mouseDoubleClickEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton) return;
    QDateTime dt = hitTimeSlot(e->pos());
    if (dt.isValid()) emit timeSlotClicked(dt);
}

// 根据鼠标位置计算对应的时间槽（日期+整点小时）
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

// 鼠标移动：显示事件详情ToolTip
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

// 滚轮事件：Ctrl+滚轮缩放行高，普通滚轮垂直滚动视图
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

// 鼠标离开：隐藏ToolTip
void TimeGridView::leaveEvent(QEvent *) { QToolTip::hideText(); }

} // namespace timemaster
