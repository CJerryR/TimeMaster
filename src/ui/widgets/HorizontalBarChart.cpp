#include "HorizontalBarChart.h"
#include "../Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace timemaster {

HorizontalBarChart::HorizontalBarChart(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(220);
    setAttribute(Qt::WA_StyledBackground, false);
    connect(&Theme::instance(), &Theme::changed, this, QOverload<>::of(&QWidget::update));
}

void HorizontalBarChart::setData(const QList<CategoryStat> &stats) {
    m_stats = stats;
    std::sort(m_stats.begin(), m_stats.end(), [](const CategoryStat &a, const CategoryStat &b) {
        return a.totalMinutes > b.totalMinutes;
    });
    update();
}

void HorizontalBarChart::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();
    auto palette = theme.palette();

    // 空数据态
    if (m_stats.isEmpty()) {
        p.setPen(theme.textPlaceholder());
        p.drawText(rect(), Qt::AlignCenter, "暂无数据");
        return;
    }

    qint64 maxV = 1;
    for (const auto &s : m_stats) if (s.totalMinutes > maxV) maxV = s.totalMinutes;

    const int n = m_stats.size();
    const int padTop = 6;
    const int padBot = 6;
    const int labelW = 60;    // 左侧类别名
    const int valueW = 70;    // 右侧时长
    const int gapH = 10;
    const int barAreaX = labelW + 8;
    const int barAreaW = std::max(40, width() - barAreaX - valueW - 8);

    int barH = (height() - padTop - padBot - (n - 1) * gapH) / std::max(1, n);
    barH = std::clamp(barH, 14, 32);

    QFont labelFont = font();
    labelFont.setPointSize(10);
    p.setFont(labelFont);

    int y = padTop;
    for (int i = 0; i < n; ++i) {
        const auto &s = m_stats[i];
        EventColor ec = categoryDefaultColor(s.category);
        QColor barColor = palette.value(ec).bg;

        // 左侧类别名
        p.setPen(theme.textPrimary());
        QRect labelRect(0, y, labelW, barH);
        p.drawText(labelRect, Qt::AlignVCenter | Qt::AlignRight, categoryLabel(s.category));

        // 条形背景（轨道）
        QRect track(barAreaX, y + (barH - 12) / 2, barAreaW, 12);
        QPainterPath trackPath;
        trackPath.addRoundedRect(track, 6, 6);
        p.fillPath(trackPath, theme.bgHover());

        // 实际条形
        double ratio = double(s.totalMinutes) / double(maxV);
        int filledW = int(track.width() * ratio);
        if (filledW < 2 && s.totalMinutes > 0) filledW = 2;
        if (filledW > 0) {
            QRect bar(track.x(), track.y(), filledW, track.height());
            QPainterPath barPath;
            barPath.addRoundedRect(bar, 6, 6);
            // 渐变让条形有质感
            QLinearGradient grad(bar.topLeft(), bar.topRight());
            grad.setColorAt(0.0, barColor.lighter(115));
            grad.setColorAt(1.0, barColor);
            p.fillPath(barPath, grad);
        }

        // 右侧时长
        qint64 m = s.totalMinutes;
        QString vText;
        if (m >= 60) {
            qint64 h = m / 60;
            qint64 mm = m % 60;
            vText = mm == 0 ? QString("%1h").arg(h) : QString("%1h%2m").arg(h).arg(mm);
        } else {
            vText = QString("%1m").arg(m);
        }
        p.setPen(theme.textSecondary());
        QRect valueRect(barAreaX + barAreaW + 4, y, valueW - 4, barH);
        p.drawText(valueRect, Qt::AlignVCenter | Qt::AlignLeft, vText);

        y += barH + gapH;
    }
}

} // namespace timemaster
