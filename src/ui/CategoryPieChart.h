//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QWidget>

namespace timemaster {

// 类别饼图/环形图：显示各类别时长占比，中空环形设计，空数据占位提示
class CategoryPieChart : public QWidget {
    Q_OBJECT
public:
    // 构造函数
    explicit CategoryPieChart(QWidget *parent = nullptr);
    // 设置统计数据并触发重绘
    void setStats(const QList<CategoryStat> &stats);

protected:
    // 绘制事件：渲染环形饼图或空数据占位符
    void paintEvent(QPaintEvent *) override;

private:
    QList<CategoryStat> m_stats;
};

} // namespace timemaster
