#pragma once

#include "../core/Types.h"
#include <QWidget>
#include <QTimer>

namespace timemaster {

class TimeGridView : public QWidget {
    Q_OBJECT
public:
    enum Mode { DayMode, WeekMode };

    explicit TimeGridView(Mode mode, QWidget *parent = nullptr);

    void setCurrentDate(const QDate &date);
    void setEvents(const QList<CalendarEvent> &events);
    void scrollToHour(int hour);

signals:
    void timeSlotClicked(const QDateTime &dt);
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
    int m_headerHeight = 66;
    int m_allDayBandHeight = 0;

    int m_scrollY = 0;
    int m_contentHeight = 0;

    QTimer m_minuteTimer;
};

} // namespace timemaster
