//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include <QDateTime>
#include "../../core/Types.h"

namespace timemaster {

// 每日趋势柱状图：按天统计事件时长的垂直条形图，今日高亮
class DailyTrendChart : public QWidget {
    Q_OBJECT
public:
    // 构造函数：设置最小高度
    explicit DailyTrendChart(QWidget *parent = nullptr);
    // 设置每日统计数据列表，触发重绘
    void setData(const QList<DailySummary> &daily);

protected:
    // 自绘事件：绘制每天垂直柱状条，突出显示今日
    void paintEvent(QPaintEvent *) override;

private:
    QList<DailySummary> m_daily;
};

} // namespace timemaster
