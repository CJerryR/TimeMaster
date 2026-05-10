#pragma once

#include <QWidget>
#include "../../core/Types.h"

class QLabel;

namespace timeplan {

class RhythmCardWidget : public QWidget {
    Q_OBJECT
public:
    explicit RhythmCardWidget(QWidget *parent = nullptr);
    void setHourlyData(const QList<HourlyBucket> &buckets);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QList<HourlyBucket> m_buckets;
    QLabel *m_title;
};

} // namespace timeplan
