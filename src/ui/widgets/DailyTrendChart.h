#pragma once

#include <QWidget>
#include <QDateTime>
#include "../../core/Types.h"

namespace timeplan {

class DailyTrendChart : public QWidget {
    Q_OBJECT
public:
    explicit DailyTrendChart(QWidget *parent = nullptr);
    void setData(const QList<DailySummary> &daily);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QList<DailySummary> m_daily;
};

} // namespace timeplan
