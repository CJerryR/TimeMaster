#pragma once

#include "../core/Types.h"
#include <QWidget>
#include <QTimer>

namespace timeplan {

/**
 * 时间网格视图（共用基础逻辑）
 * 模式：Day / Week
 * 自绘：左侧时间标尺 + 顶部日期表头 + 中央事件层 + "现在"红线
 *
 * 重叠事件采用列布局：把同一时段的事件按贪心算法分配列，等宽显示
 */
class TimeGridView : public QWidget {
    Q_OBJECT
public:
    enum Mode { DayMode, WeekMode };

    explicit TimeGridView(Mode mode, QWidget *parent = nullptr);

    void setCurrentDate(const QDate &date);
    void setEvents(const QList<CalendarEvent> &events);
    void scrollToHour(int hour);

signals:
    void timeSlotClicked(const QDateTime &dt);   // 双击空白
    void eventClicked(const CalendarEvent &event);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void leaveEvent(QEvent *) override;

private slots:
    void onMinuteTick();

private:
    struct EventLayout {
        QRect rect;
        CalendarEvent event;
    };

    void rebuildLayout();
    QList<QDate> visibleDays() const;
    QList<EventLayout> layoutDayEvents(const QDate &date, const QRect &dayCol);
    QDateTime hitTimeSlot(const QPoint &pos) const;

    Mode m_mode;
    QDate m_currentDate;
    QList<CalendarEvent> m_events;
    QList<EventLayout> m_eventRects;

    int m_hourHeight = 56;
    int m_timeGutter = 56;
    int m_headerHeight = 60;
    int m_allDayBandHeight = 0;

    int m_scrollY = 0;
    int m_contentHeight = 0;

    QTimer m_minuteTimer;
};

} // namespace timeplan
