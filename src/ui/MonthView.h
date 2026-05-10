#pragma once

#include "../core/Types.h"
#include <QWidget>

namespace timeplan {

/**
 * 月视图（自绘）
 * - 6 行 × 7 列网格
 * - 每个单元格显示日期 + 至多 N 条事件，多余以 "+N 更多" 显示
 * - 鼠标悬浮高亮、点击单元格新建、点击事件编辑
 */
class MonthView : public QWidget {
    Q_OBJECT
public:
    explicit MonthView(QWidget *parent = nullptr);

    void setCurrentDate(const QDate &date);
    void setEvents(const QList<CalendarEvent> &events);

signals:
    void dateClicked(const QDate &date);
    void eventClicked(const CalendarEvent &event);
    void overflowClicked(const QDate &date, const QList<CalendarEvent> &allEventsThatDay);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

private:
    struct CellLayout {
        QRect rect;
        QDate date;
        bool isCurrentMonth;
        bool isToday;
    };
    struct EventLayout {
        QRect rect;
        CalendarEvent event;
    };
    struct OverflowLayout {
        QRect rect;
        QDate date;
        QList<CalendarEvent> events;
    };

    void rebuildLayout();
    QList<CalendarEvent> eventsForDay(const QDate &d) const;
    QList<QDate> buildDays() const;

    QDate m_currentDate;
    QList<CalendarEvent> m_events;
    QList<CellLayout> m_cells;
    QList<EventLayout> m_eventRects;
    QList<OverflowLayout> m_overflowRects;

    int m_hoverIndex = -1;
};

} // namespace timeplan
