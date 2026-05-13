//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include "../../core/Types.h"

class QLabel;

namespace timemaster {

// 24小时节奏热力图：每小时段总时长的垂直柱状图，高峰时段红色标注
class RhythmCardWidget : public QWidget {
    Q_OBJECT
public:
    // 构造函数：初始化标题和布局
    explicit RhythmCardWidget(QWidget *parent = nullptr);
    // 设置每小时统计数据列表，触发重绘
    void setHourlyData(const QList<HourlyBucket> &buckets);

protected:
    // 自绘事件：绘制24小时垂直柱状图，高峰时段标红
    void paintEvent(QPaintEvent *) override;

private:
    QList<HourlyBucket> m_buckets;
    QLabel *m_title;
};

} // namespace timemaster
