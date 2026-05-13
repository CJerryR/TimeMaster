//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QWidget>

namespace timemaster {

// 月视图网格（6周x7天）：绘制日期格、事件色条、今日高亮、悬停效果、+N溢出指示
class MonthView : public QWidget {
    Q_OBJECT
public:
    // 构造函数：启用鼠标追踪，连接语言/周起始日变化信号
    explicit MonthView(QWidget *parent = nullptr);

    // 设置当前显示月份，触发重绘
    void setCurrentDate(const QDate &date);
    // 设置事件列表，触发重绘
    void setEvents(const QList<CalendarEvent> &events);

signals:
    // 点击日期信号
    void dateClicked(const QDate &date);
    // 点击事件信号
    void eventClicked(const CalendarEvent &event);
    // 点击"+N"溢出指示信号
    void overflowClicked(const QDate &date, const QList<CalendarEvent> &allEventsThatDay);

protected:
    // 绘制：网格线、日期数字、事件色条、今日高亮、悬停效果、+N溢出文本
    void paintEvent(QPaintEvent *) override;
    // 尺寸变化时重建布局
    void resizeEvent(QResizeEvent *) override;
    // 鼠标按下：检测事件点击或溢出点击或日期点击
    void mousePressEvent(QMouseEvent *) override;
    // 鼠标移动：更新悬停索引并显示 ToolTip
    void mouseMoveEvent(QMouseEvent *) override;
    // 鼠标离开：清除悬停状态
    void leaveEvent(QEvent *) override;
    // 鼠标双击：触发日期点击信号
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

    // 重建单元格/事件矩形/溢出指示的布局
    void rebuildLayout();
    // 查询指定日期的事件列表，按全天优先+时间排序
    QList<CalendarEvent> eventsForDay(const QDate &d) const;
    // 构建当前月视图需要的42天日期序列（含前后月填充）
    QList<QDate> buildDays() const;

    QDate m_currentDate;
    QList<CalendarEvent> m_events;
    QList<CellLayout> m_cells;
    QList<EventLayout> m_eventRects;
    QList<OverflowLayout> m_overflowRects;

    int m_hoverIndex = -1;
};

} // namespace timemaster
