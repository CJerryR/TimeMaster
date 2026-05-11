#pragma once

#include <QWidget>
#include <QList>
#include "../../core/Types.h"

namespace timemaster {

/**
 * 每日趋势图：横轴为日期，纵轴为总时长（分钟）。
 * 自绘折线 + 半透明面积。
 */
class DailyTrendChart : public QWidget {
    Q_OBJECT
public:
    explicit DailyTrendChart(QWidget *parent = nullptr);
    void setData(const QList<DailySummary> &daily);

    QSize sizeHint() const override { return QSize(720, 220); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<DailySummary> m_daily;
};

} // namespace timemaster
