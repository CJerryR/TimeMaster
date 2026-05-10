// ============================================================
// DailyTrendChart.h
// 每日日程趋势 mini 柱状图：x 轴日期(每周一显示)，y 轴日程数
// 高亮最高条，底部显示均值参考线
// 对应网页版 DailyTrendChart 组件
// ============================================================
#pragma once
#include <QWidget>
#include <QString>
#include <QVector>
#include <QDate>

namespace cadence {

struct TrendPoint {
    QDate date;
    int count;
};

class DailyTrendChart : public QWidget {
    Q_OBJECT
public:
    explicit DailyTrendChart(QWidget* parent = nullptr);
    void setData(const QVector<TrendPoint>& data);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QVector<TrendPoint> m_data;
};

} // namespace cadence
