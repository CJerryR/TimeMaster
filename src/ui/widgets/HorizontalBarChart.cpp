#include "HorizontalBarChart.h"
#include "../Theme.h"
#include "../../core/Types.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace timeplan {

HorizontalBarChart::HorizontalBarChart(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(180);
}

void HorizontalBarChart::setData(const QList<CategoryStat> &stats) {
    m_stats = stats;
    update();
}

void HorizontalBarChart::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();
    auto pal = theme.palette();

    if (m_stats.isEmpty()) {
        p.setPen(theme.textPlaceholder());
        p.drawText(rect(), Qt::AlignCenter, "暂无数据");
        return;
    }

    int barH = 32;
    int gap = 10;
    int totalH = m_stats.size() * (barH + gap) - gap;
    int startY = (height() - totalH) / 2;

    QFont nameFont = font();
    nameFont.setPointSize(11);
    p.setFont(nameFont);

    qint64 maxMin = 0;
    for (const auto &s : m_stats)
        if (s.totalMinutes > maxMin) maxMin = s.totalMinutes;
    if (maxMin <= 0) maxMin = 1;

    int labelW = 80;
    int barAreaW = width() - labelW - 70;
    int barLeft = labelW + 10;

    for (int i = 0; i < m_stats.size(); ++i) {
        const auto &s = m_stats[i];
        int y = startY + i * (barH + gap);

        QColor c = pal[categoryDefaultColor(s.category)].text;

        p.setPen(theme.textPrimary());
        p.drawText(QRect(0, y, labelW, barH), Qt::AlignRight | Qt::AlignVCenter,
                   categoryLabel(s.category));

        int barW = int(double(s.totalMinutes) / maxMin * barAreaW);
        if (barW < 4) barW = 4;
        QRect barR(barLeft, y, barW, barH);

        QPainterPath path;
        path.addRoundedRect(barR, 6, 6);
        p.fillPath(path, c);

        p.setPen(theme.textPrimary());
        QRect valR(barLeft + barW + 8, y, 60, barH);
        qint64 h = s.totalMinutes / 60;
        qint64 m = s.totalMinutes % 60;
        QString val = h > 0 ? QString("%1h %2m").arg(h).arg(m) : QString("%1m").arg(m);
        p.drawText(valR, Qt::AlignLeft | Qt::AlignVCenter, val);
    }
}

} // namespace timeplan
