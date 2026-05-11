#pragma once

#include "../core/Types.h"
#include <QWidget>

namespace timemaster {

class CategoryPieChart : public QWidget {
    Q_OBJECT
public:
    explicit CategoryPieChart(QWidget *parent = nullptr);
    void setStats(const QList<CategoryStat> &stats);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QList<CategoryStat> m_stats;
};

} // namespace timemaster
