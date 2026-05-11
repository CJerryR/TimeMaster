#pragma once

#include <QWidget>
#include <QLabel>
#include <QList>
#include "../../core/Types.h"

namespace timemaster {

/**
 * 24 小时节奏卡片：展示每个小时段的活动时长。
 * 顶部标题 + 副标题，下方是 24 个垂直柱条。
 */
class RhythmCardWidget : public QWidget {
    Q_OBJECT
public:
    explicit RhythmCardWidget(QWidget *parent = nullptr);
    void setHourlyData(const QList<HourlyBucket> &buckets);

    QSize sizeHint() const override { return QSize(360, 220); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void applyTheme();
    void recomputeSubtitle();

    QLabel *m_title = nullptr;
    QLabel *m_subtitle = nullptr;
    QList<HourlyBucket> m_buckets;
};

} // namespace timemaster
