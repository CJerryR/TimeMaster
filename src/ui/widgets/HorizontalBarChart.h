//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include <QString>
#include "../../core/Types.h"

namespace timemaster {

// 横向条形图：各类别时长占比，彩色圆角条+数值标签
class HorizontalBarChart : public QWidget {
    Q_OBJECT
public:
    // 构造函数：设置最小高度
    explicit HorizontalBarChart(QWidget *parent = nullptr);
    // 设置统计数据列表，触发重绘
    void setData(const QList<CategoryStat> &stats);

protected:
    // 自绘事件：绘制横向圆角条形图及类别名称和时长标签
    void paintEvent(QPaintEvent *) override;

private:
    QList<CategoryStat> m_stats;
};

} // namespace timemaster
