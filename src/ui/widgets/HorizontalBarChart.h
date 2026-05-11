#pragma once

#include <QWidget>
#include <QList>
#include "../../core/Types.h"

namespace timemaster {

/**
 * 横向条形图：显示各类别的总时长。
 * 自绘 (paintEvent)，使用 Theme 色板。
 */
class HorizontalBarChart : public QWidget {
    Q_OBJECT
public:
    explicit HorizontalBarChart(QWidget *parent = nullptr);
    void setData(const QList<CategoryStat> &stats);

    QSize sizeHint() const override { return QSize(360, 280); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<CategoryStat> m_stats;
};

} // namespace timemaster
