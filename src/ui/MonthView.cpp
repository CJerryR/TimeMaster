#include "MonthView.h"
#include "Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>

namespace timeplan {

namespace {
constexpr int HEADER_HEIGHT = 28;
constexpr int CELL_PADDING = 4;
constexpr int EVENT_BAR_HEIGHT = 18;
constexpr int EVENT_BAR_GAP = 2;
constexpr int DATE_TEXT_HEIGHT = 30;
}

MonthView::MonthView(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    m_currentDate = QDate::currentDate();
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
    int dayOfWeek = firstOfMonth.dayOfWeek() % 7; // 周日=0
    QDate gridStart = firstOfMonth.addDays(-dayOfWeek);
    for (int i = 0; i < 42; ++i) {
        days << gridStart.addDays(i);
    }
    return days;
}

QList<CalendarEvent> MonthView::eventsForDay(const QDate &d) const {
    QList<CalendarEvent> list;
    for (const auto &e : m_events) {
        QDate s = e.startDate.date();
        QDate end = e.endDate.date();
        if (d >= s && d <= end) list.append(e);
    }
    // 全天优先，再按开始时间
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

            // 事件最多显示 N 条
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

    p.fillRect(rect(), theme.bgPage());

    // 周表头
    static const char *weekdays[] = {"日","一","二","三","四","五","六"};
    QFont headerFont = font();
    headerFont.setPointSize(10);
    headerFont.setWeight(QFont::DemiBold);
    p.setFont(headerFont);
    double cellW = double(width()) / 7.0;

    for (int i = 0; i < 7; ++i) {
        QRect r(qRound(i * cellW), 0, qRound(cellW + 1), HEADER_HEIGHT);
        bool weekend = (i == 0 || i == 6);
        p.setPen(weekend ? theme.brand() : theme.textSecondary());
        p.drawText(r, Qt::AlignCenter, weekdays[i]);
    }

    // 表头底部分隔线
    p.setPen(theme.stroke());
    p.drawLine(0, HEADER_HEIGHT, width(), HEADER_HEIGHT);

    if (m_cells.isEmpty()) {
        rebuildLayout();
    }

    // 单元格
    QFont dateFont = font();
    dateFont.setPointSize(11);
    QFont dateBigFont = font();
    dateBigFont.setPointSize(12);
    dateBigFont.setWeight(QFont::Bold);

    for (int i = 0; i < m_cells.size(); ++i) {
        const auto &cell = m_cells[i];

        // 单元格背景
        if (cell.isToday) {
            p.fillRect(cell.rect, theme.todayHighlight());
        } else if (i == m_hoverIndex) {
            p.fillRect(cell.rect, theme.bgHover());
        }

        // 网格线
        p.setPen(QPen(theme.stroke(), 1));
        p.drawLine(cell.rect.right(), cell.rect.top(),
                   cell.rect.right(), cell.rect.bottom());
        p.drawLine(cell.rect.left(), cell.rect.bottom(),
                   cell.rect.right(), cell.rect.bottom());

        // 日期数字
        QRect dateRect(cell.rect.left() + 6,
                       cell.rect.top() + 4,
                       cell.rect.width() - 12,
                       DATE_TEXT_HEIGHT - 4);

        QColor dateColor;
        if (cell.isToday) {
            dateColor = theme.brand();
        } else if (!cell.isCurrentMonth) {
            dateColor = theme.textPlaceholder();
        } else {
            int dow = cell.date.dayOfWeek() % 7;
            dateColor = (dow == 0 || dow == 6) ? theme.danger() : theme.textPrimary();
        }
        p.setPen(dateColor);

        if (cell.isToday) {
            p.setFont(dateBigFont);
            // 圆形 today 高亮
            int diam = 22;
            QRect circle(dateRect.left(), dateRect.top() + (dateRect.height() - diam) / 2, diam, diam);
            p.setBrush(theme.brand());
            p.setPen(Qt::NoPen);
            p.drawEllipse(circle);
            p.setPen(Qt::white);
            p.drawText(circle, Qt::AlignCenter, QString::number(cell.date.day()));
        } else {
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
        path.addRoundedRect(er.rect, 4, 4);
        p.fillPath(path, c.bg);

        // 左侧色条
        QRect bar(er.rect.left(), er.rect.top(), 3, er.rect.height());
        p.fillRect(bar, c.text);

        // 文字
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

    // 溢出 +N
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
        QString label = QString("+ %1 更多").arg(extra);
        p.drawText(o.rect, Qt::AlignCenter, label);
    }
}

void MonthView::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton) return;

    // 优先检查事件
    for (const auto &er : m_eventRects) {
        if (er.rect.contains(e->pos())) {
            emit eventClicked(er.event);
            return;
        }
    }

    // 检查溢出按钮
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

    // tooltip for events
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

} // namespace timeplan
