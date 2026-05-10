// ============================================================
// HorizontalBarChart.h
// 横向柱状图：左侧标签、中间彩色条、右侧数值
// 对应网页版 BarChart 组件
// ============================================================
#pragma once
#include <QWidget>
#include <QString>
#include <QColor>
#include <QVector>

namespace cadence {

struct BarItem {
    QString label;   // "工作"
    double value;    // 已转换的展示数值（如小时）
    QString suffix;  // "h" 或 "min"，附在数值后
    QColor color;
};

class HorizontalBarChart : public QWidget {
    Q_OBJECT
public:
    explicit HorizontalBarChart(QWidget* parent = nullptr);
    void setData(const QVector<BarItem>& items);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QVector<BarItem> m_items;
};

} // namespace cadence
