#pragma once

#include <QWidget>
#include <QString>
#include "../../core/Types.h"

namespace timeplan {

class HorizontalBarChart : public QWidget {
    Q_OBJECT
public:
    explicit HorizontalBarChart(QWidget *parent = nullptr);
    void setData(const QList<CategoryStat> &stats);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QList<CategoryStat> m_stats;
};

} // namespace timeplan
