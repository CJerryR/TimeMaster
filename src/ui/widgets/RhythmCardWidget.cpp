#include "RhythmCardWidget.h"
#include "../Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>
#include <QtMath>

namespace timeplan {

RhythmCardWidget::RhythmCardWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(120);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    m_title = new QLabel("每日节奏");
    m_title->setProperty("class", "subtitle");
    lay->addWidget(m_title);
}

void RhythmCardWidget::setHourlyData(const QList<HourlyBucket> &buckets) {
    m_buckets = buckets;
    update();
}

void RhythmCardWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();
    int topOffset = 26;
    QRect chartR(rect().left() + 8, rect().top() + topOffset,
                 rect().width() - 16, rect().height() - topOffset - 8);

    qint64 maxMin = 0;
    for (const auto &b : m_buckets)
        if (b.totalMinutes > maxMin) maxMin = b.totalMinutes;
    if (maxMin <= 0) {
        p.setPen(theme.textPlaceholder());
        p.drawText(chartR, Qt::AlignCenter, "暂无数据");
        return;
    }

    int n = m_buckets.size();
    if (n <= 0) return;

    double barPad = 0.25;
    double slotW = double(chartR.width()) / n;
    double barW = slotW * (1.0 - barPad);

    QColor barColor = theme.brand();
    QColor peakColor = theme.danger();

    for (int i = 0; i < n; ++i) {
        double x = chartR.left() + slotW * i + slotW * barPad / 2.0;
        double ratio = double(m_buckets[i].totalMinutes) / maxMin;
        double h = ratio * chartR.height();
        if (h < 2 && m_buckets[i].totalMinutes > 0) h = 2;
        double y = chartR.bottom() - h;

        QRectF barR(x, y, barW, h);
        QPainterPath pp;
        pp.addRoundedRect(barR, 3, 3);

        bool isPeak = (ratio >= 0.9);
        p.fillPath(pp, isPeak ? peakColor : barColor);

        if (m_buckets[i].hour % 6 == 0) {
            p.setPen(theme.textSecondary());
            QFont f = font();
            f.setPointSize(8);
            p.setFont(f);
            QRectF lr(x - 8, chartR.bottom() + 2, slotW + 8, 14);
            p.drawText(lr, Qt::AlignCenter, QString::number(m_buckets[i].hour));
        }
    }
}

} // namespace timeplan
