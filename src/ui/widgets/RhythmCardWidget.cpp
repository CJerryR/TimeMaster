#include "RhythmCardWidget.h"
#include "../Theme.h"
#include "../../core/I18n.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>
#include <QtMath>

namespace timemaster {

RhythmCardWidget::RhythmCardWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(160);   // V4.2: was 120, give chart + labels room

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    m_title = new QLabel(I18n::t("widget.rhythm"));
    m_title->setProperty("class", "subtitle");
    lay->addWidget(m_title);
    connect(&I18n::instance(), &I18n::languageChanged, this, [this]{
        if (m_title) m_title->setText(I18n::t("widget.rhythm"));
        update();
    });
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
    // FIX (V4.2): bottom 边距从 8 增加到 24，给 hour 数字标签 (0/6/12/18) 留出
    // 完整的绘制区域，不再被 widget 边缘裁切。
    const int bottomLabelArea = 22;
    QRect chartR(rect().left() + 8, rect().top() + topOffset,
                 rect().width() - 16, rect().height() - topOffset - bottomLabelArea);

    qint64 maxMin = 0;
    for (const auto &b : m_buckets)
        if (b.totalMinutes > maxMin) maxMin = b.totalMinutes;
    if (maxMin <= 0) {
        p.setPen(theme.textPlaceholder());
        p.drawText(chartR, Qt::AlignCenter, timemaster::I18n::t("widget.no_data"));
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
            f.setPointSize(10);  // ~13px (hour markers)
            p.setFont(f);
            QRectF lr(x - 8, chartR.bottom() + 4, slotW + 8, 18);
            p.drawText(lr, Qt::AlignCenter, QString::number(m_buckets[i].hour));
        }
    }
}

} // namespace timemaster
