//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QWidget>
#include <QTimer>

namespace timemaster {

// 日/周时间网格视图：时间轴纵列、事件色块布局、今日高亮、当前时间红线、Ctrl+滚轮缩放
class TimeGridView : public QWidget {
    Q_OBJECT
public:
    enum Mode { DayMode, WeekMode };

    // 构造函数：初始化分钟定时器，连接语言/周起始日变化信号
    explicit TimeGridView(Mode mode, QWidget *parent = nullptr);

    // 设置当前日期，触发重绘
    void setCurrentDate(const QDate &date);
    // 设置事件列表，触发重绘
    void setEvents(const QList<CalendarEvent> &events);
    // 滚动到指定小时位置
    void scrollToHour(int hour);

signals:
    // 点击时间槽信号
    void timeSlotClicked(const QDateTime &dt);
    // 点击事件信号
    void eventClicked(const CalendarEvent &event);

protected:
    // 绘制：时间轴、日期头部、事件色块、当前时间红线、今日列高亮
    void paintEvent(QPaintEvent *) override;
    // 尺寸变化时重建布局
    void resizeEvent(QResizeEvent *) override;
    // 鼠标按下：检测事件点击
    void mousePressEvent(QMouseEvent *) override;
    // 鼠标双击：检测时间槽点击，触发新建事件
    void mouseDoubleClickEvent(QMouseEvent *) override;
    // 鼠标移动：显示事件 ToolTip
    void mouseMoveEvent(QMouseEvent *) override;
    // 滚轮事件：Ctrl+滚轮缩放行高，普通滚轮滚动视图
    void wheelEvent(QWheelEvent *) override;
    // 鼠标离开：隐藏 ToolTip
    void leaveEvent(QEvent *) override;

private slots:
    // 每分钟触发：更新时间红线
    void onMinuteTick();

private:
    struct EventLayout {
        QRect rect;
        CalendarEvent event;
    };

    // 重建所有事件矩形和全天事件带的布局
    void rebuildLayout();
    // 返回当前视图可见的日期列表（日模式2天，周模式7天）
    QList<QDate> visibleDays() const;
    // 对单日事件进行列分配和坐标计算，返回矩形列表
    QList<EventLayout> layoutDayEvents(const QDate &date, const QRect &dayCol);
    // 根据鼠标位置计算对应的时间槽（日期+小时）
    QDateTime hitTimeSlot(const QPoint &pos) const;

    Mode m_mode;
    QDate m_currentDate;
    QList<CalendarEvent> m_events;
    QList<EventLayout> m_eventRects;

    int m_hourHeight = 56;
    int m_timeGutter = 56;
    int m_headerHeight = 72;     // V4.3.2 #2 — 66 → 72 配合红圈下移 5px 后底部留呼吸
    int m_allDayBandHeight = 0;

    int m_scrollY = 0;
    int m_contentHeight = 0;

    QTimer m_minuteTimer;
};

} // namespace timemaster
