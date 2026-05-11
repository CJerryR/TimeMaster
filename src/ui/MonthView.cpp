#include "MonthView.h"
#include "Theme.h"
#include "../core/I18n.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>

namespace timemaster {

namespace {
// V4 § 8.4: padding 4 -> 8; date text height 32 -> 40
constexpr int HEADER_HEIGHT   = 32;
constexpr int CELL_PADDING    = 8;
constexpr int EVENT_BAR_HEIGHT = 19;
constexpr int EVENT_BAR_GAP   = 3;
constexpr int DATE_TEXT_HEIGHT = 40;

constexpr int TODAY_DIAMETER  = 30;     // V4 § 3.4: 26 -> 30
}

MonthView::MonthView(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setAttribute(Qt::WA_StyledBackground, false);
    m_currentDate = QDate::currentDate();
    // 语言变化时表头要重画
    connect(&I18n::instance(), &I18n::languageChanged, this, [this]{ update(); });
}

void MonthView::setCurrentDate(const QDate &date) {
    m_currentDate = date;
    rebuildLayout();
    update();
}

void MonthView::setEvents(const QList<CalendarEvent> &events) {
    m_events = events;
    rebuildLayout();
    update();
}

QList<QDate> MonthView::buildDays() const {
    QList<QDate> days;
    QDate firstOfMonth(m_currentDate.year(), m_currentDate.month(), 1);
    int dayOfWeek = firstOfMonth.dayOfWeek() - 1;  // Mon=0, Sun=6
    QDate gridStart = firstOfMonth.addDays(-dayOfWeek);
    for (int i = 0; i < 42; ++i) days << gridStart.addDays(i);
    return days;
}

QList<CalendarEvent> MonthView::eventsForDay(const QDate &d) const {
    QList<CalendarEvent> list;
    for (const auto &e : m_events) {
        QDate s = e.startDate.date();
        QDate end = e.endDate.date();
        if (d >= s && d <= end) list.append(e);
    }
    std::sort(list.begin(), list.end(), [](const CalendarEvent &a, const CalendarEvent &b) {
        if (a.allDay != b.allDay) return a.allDay > b.allDay;
        return a.startDate < b.startDate;
    });
    return list;
}

void MonthView::rebuildLayout() {
    m_cells.clear();
    m_eventRects.clear();
    m_overflowRects.clear();

    if (width() <= 0 || height() <= 0) return;

    QList<QDate> days = buildDays();
    QDate today = QDate::currentDate();
    int currentMonth = m_currentDate.month();

    int gridY = HEADER_HEIGHT;
    int gridH = height() - HEADER_HEIGHT;
    if (gridH <= 0) return;

    double cellW = double(width()) / 7.0;
    double cellH = double(gridH) / 6.0;

    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            int idx = row * 7 + col;
            QDate d = days[idx];
            QRect r(qRound(col * cellW), qRound(gridY + row * cellH),
                    qRound(cellW + 1), qRound(cellH + 1));
            CellLayout cell;
            cell.rect = r;
            cell.date = d;
            cell.isCurrentMonth = (d.month() == currentMonth);
            cell.isToday = (d == today);
            m_cells.append(cell);

            QList<CalendarEvent> dayEvents = eventsForDay(d);
            int maxSlots = qMax(0, (r.height() - DATE_TEXT_HEIGHT - CELL_PADDING * 2)
                                / (EVENT_BAR_HEIGHT + EVENT_BAR_GAP));
            maxSlots = qMin(maxSlots, 4);
            int showCount = qMin(maxSlots, dayEvents.size());
            if (dayEvents.size() > maxSlots) {
                showCount = maxSlots - 1;
                if (showCount < 0) showCount = 0;
            }

            for (int j = 0; j < showCount; ++j) {
                QRect er(r.left() + CELL_PADDING,
                         r.top() + DATE_TEXT_HEIGHT + j * (EVENT_BAR_HEIGHT + EVENT_BAR_GAP),
                         r.width() - 2 * CELL_PADDING,
                         EVENT_BAR_HEIGHT);
                m_eventRects.append({er, dayEvents[j]});
            }

            if (dayEvents.size() > showCount) {
                int j = showCount;
                QRect or_(r.left() + CELL_PADDING,
                          r.top() + DATE_TEXT_HEIGHT + j * (EVENT_BAR_HEIGHT + EVENT_BAR_GAP),
                          r.width() - 2 * CELL_PADDING,
                          EVENT_BAR_HEIGHT);
                m_overflowRects.append({or_, d, dayEvents});
            }
        }
    }
}

void MonthView::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    auto &theme = Theme::instance();
    auto pal = theme.palette();

    p.fillRect(rect(), Qt::transparent);

    // 周表头：Mon..Sun
    static const char *keys[] = {
        "calendar.month.weekday.mon", "calendar.month.weekday.tue",
        "calendar.month.weekday.wed", "calendar.month.weekday.thu",
        "calendar.month.weekday.fri", "calendar.month.weekday.sat",
        "calendar.month.weekday.sun"
    };
    QFont headerFont = font();
    headerFont.setPointSize(11);
    headerFont.setWeight(QFont::DemiBold);
    p.setFont(headerFont);
    double cellW = double(width()) / 7.0;

    for (int i = 0; i < 7; ++i) {
        QRect r(qRound(i * cellW), 0, qRound(cellW + 1), HEADER_HEIGHT);
        // V4 § 8.1: 周末统一用 textSecondary 暖灰，不要 brand 不要 danger
        p.setPen(theme.textSecondary());
        p.drawText(r, Qt::AlignCenter, I18n::t(keys[i]));
    }

    // V4 § 8.3：网格线 alpha 极淡（Theme.stroke 已降至 0.08）
    p.setPen(theme.stroke());
    p.drawLine(0, HEADER_HEIGHT, width(), HEADER_HEIGHT);

    if (m_cells.isEmpty()) rebuildLayout();

    QFont dateFont = font();
    dateFont.setPointSize(11);   // ~14px @ 96dpi
    dateFont.setWeight(QFont::Medium);  // V4 § 3.4

    QFont todayFont = font();
    todayFont.setPointSize(11);
    todayFont.setWeight(QFont::DemiBold);  // 600

    for (int i = 0; i < m_cells.size(); ++i) {
        const auto &cell = m_cells[i];

        // V4 § 8.2：删除今日背景色块，只留实心圆
        if (i == m_hoverIndex && !cell.isToday) {
            p.fillRect(cell.rect, theme.bgHover());
        }

        p.setPen(QPen(theme.stroke(), 1));
        p.drawLine(cell.rect.right(), cell.rect.top(),
                   cell.rect.right(), cell.rect.bottom());
        p.drawLine(cell.rect.left(), cell.rect.bottom(),
                   cell.rect.right(), cell.rect.bottom());

        // 日期数字：左上角 (12, 10) padding (V4 § 3.4)
        QRect dateRect(cell.rect.left() + 12,
                       cell.rect.top() + 10,
                       cell.rect.width() - 24,
                       DATE_TEXT_HEIGHT - 10);

        QColor dateColor;
        if (cell.isToday) {
            dateColor = Qt::white;
        } else if (!cell.isCurrentMonth) {
            dateColor = theme.textPlaceholder();
        } else {
            int dow = cell.date.dayOfWeek() % 7;
            // V4 § 8.1: 周末用 textSecondary（不 brand 不 danger）
            dateColor = (dow == 0 || dow == 6) ? theme.textSecondary() : theme.textPrimary();
        }

        if (cell.isToday) {
            p.setFont(todayFont);
            int diam = TODAY_DIAMETER;
            QRect circle(dateRect.left(), dateRect.top() + (dateRect.height() - diam) / 2,
                         diam, diam);
            p.setBrush(theme.brand());
            p.setPen(Qt::NoPen);
            p.drawEllipse(circle);
            p.setPen(Qt::white);
            p.drawText(circle, Qt::AlignCenter, QString::number(cell.date.day()));
        } else {
            p.setPen(dateColor);
            p.setFont(dateFont);
            p.drawText(dateRect, Qt::AlignLeft | Qt::AlignVCenter,
                       QString::number(cell.date.day()));
        }
    }

    // 事件条
    QFont eventFont = font();
    eventFont.setPointSize(10);
    p.setFont(eventFont);

    for (const auto &er : m_eventRects) {
        auto &c = pal[er.event.color];
        QPainterPath path;
        path.addRoundedRect(er.rect, 5, 5);
        p.fillPath(path, c.bg);

        // V4 § 8.5：左侧色条 3px -> 2px
        QRect bar(er.rect.left() + 1, er.rect.top() + 2, 2, qMax(0, er.rect.height() - 4));
        QPainterPath barPath;
        barPath.addRoundedRect(bar, 1.0, 1.0);
        p.fillPath(barPath, c.text);

        QRect tr(er.rect.left() + 8, er.rect.top(),
                 er.rect.width() - 12, er.rect.height());
        p.setPen(c.text);

        QString label = er.event.title;
        if (!er.event.allDay) {
            label = er.event.startDate.toString("HH:mm") + " " + label;
        }
        QFontMetrics fm(eventFont);
        label = fm.elidedText(label, Qt::ElideRight, tr.width());
        p.drawText(tr, Qt::AlignLeft | Qt::AlignVCenter, label);
    }

    p.setPen(theme.textSecondary());
    QFont moreFont = font();
    moreFont.setPointSize(10);
    moreFont.setWeight(QFont::DemiBold);
    p.setFont(moreFont);
    for (const auto &o : m_overflowRects) {
        int totalForDay = o.events.size();
        int shown = 0;
        for (const auto &er : m_eventRects) {
            QDate s = er.event.startDate.date();
            QDate e = er.event.endDate.date();
            if (o.date >= s && o.date <= e) shown++;
        }
        int extra = totalForDay - shown;
        if (extra <= 0) continue;
        QString label = I18n::t("calendar.month.more_fmt").arg(extra);
        p.drawText(o.rect, Qt::AlignCenter, label);
    }
}

void MonthView::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton) return;
    for (const auto &er : m_eventRects) {
        if (er.rect.contains(e->pos())) {
            emit eventClicked(er.event);
            return;
        }
    }
    for (const auto &o : m_overflowRects) {
        if (o.rect.contains(e->pos())) {
            emit overflowClicked(o.date, o.events);
            return;
        }
    }
}

void MonthView::mouseDoubleClickEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton) return;
    for (const auto &cell : m_cells) {
        if (cell.rect.contains(e->pos())) {
            emit dateClicked(cell.date);
            return;
        }
    }
}

void MonthView::mouseMoveEvent(QMouseEvent *e) {
    int newHover = -1;
    for (int i = 0; i < m_cells.size(); ++i) {
        if (m_cells[i].rect.contains(e->pos())) { newHover = i; break; }
    }
    if (newHover != m_hoverIndex) {
        m_hoverIndex = newHover;
        update();
    }

    for (const auto &er : m_eventRects) {
        if (er.rect.contains(e->pos())) {
            QString tip = er.event.title;
            if (!er.event.allDay) {
                tip += "\n" + er.event.startDate.toString("MM-dd HH:mm")
                    + " - " + er.event.endDate.toString("HH:mm");
            }
            QToolTip::showText(e->globalPosition().toPoint(), tip, this);
            return;
        }
    }
    QToolTip::hideText();
}

void MonthView::leaveEvent(QEvent *) {
    m_hoverIndex = -1;
    update();
}

void MonthView::resizeEvent(QResizeEvent *) {
    rebuildLayout();
}

} // namespace timemaster
