#include "HorizontalBarChart.h"
#include "../Theme.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <algorithm>

namespace cadence {

HorizontalBarChart::HorizontalBarChart(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(180);
}

QSize HorizontalBarChart::sizeHint() const {
    int rows = std::max(3, m_items.size());
    return QSize(360, 24 + rows * 32);
}

void HorizontalBarChart::setData(const QVector<BarItem>& items) {
    m_items = items;
    updateGeometry();
    update();
}

void HorizontalBarChart::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& th = Theme::instance();

    if (m_items.isEmpty()) {
        p.setPen(th.textPlaceholder());
        QFont f = font(); f.setPixelSize(13); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "暂无数据");
        return;
    }

    double maxVal = 0;
    for (const auto& it : m_items) maxVal = std::max(maxVal, it.value);
    if (maxVal <= 0) maxVal = 1;

    QFont labelFont = font(); labelFont.setPixelSize(12);
    QFont valueFont = font(); valueFont.setPixelSize(12); valueFont.setWeight(QFont::DemiBold);
    QFontMetrics labelFm(labelFont);
    QFontMetrics valueFm(valueFont);

    // 左侧标签宽度（最长那条）
    int labelW = 0;
    for (const auto& it : m_items)
        labelW = std::max(labelW, labelFm.horizontalAdvance(it.label));
    labelW = std::min(labelW + 12, 100);

    // 右侧数值宽度
    int valueW = 0;
    for (const auto& it : m_items) {
        QString s = QString::number(it.value, 'f', 1) + it.suffix;
        valueW = std::max(valueW, valueFm.horizontalAdvance(s));
    }
    valueW += 12;

    const int rowH = 28;
    const int barH = 14;
    const int gap = 8;
    const int barX = labelW;
    const int barMaxW = width() - labelW - valueW - 8;
    if (barMaxW <= 10) return;

    int y = 12;
    for (const auto& it : m_items) {
        // 标签
        p.setFont(labelFont);
        p.setPen(th.textSecondary());
        p.drawText(QRect(0, y, labelW - 12, rowH),
                   Qt::AlignRight | Qt::AlignVCenter, it.label);

        // 背景条
        const QRect bgR(barX, y + (rowH - barH) / 2, barMaxW, barH);
        p.setPen(Qt::NoPen);
        p.setBrush(th.bgComponent());
        p.drawRoundedRect(bgR, barH / 2, barH / 2);

        // 数据条
        int w = static_cast<int>(barMaxW * (it.value / maxVal));
        if (w > 0) {
            const QRect fgR(barX, y + (rowH - barH) / 2, w, barH);
            p.setBrush(it.color);
            p.drawRoundedRect(fgR, barH / 2, barH / 2);
        }

        // 数值
        p.setFont(valueFont);
        p.setPen(th.textPrimary());
        QString s = QString::number(it.value, 'f', 1) + it.suffix;
        p.drawText(QRect(barX + barMaxW + 4, y, valueW, rowH),
                   Qt::AlignLeft | Qt::AlignVCenter, s);

        y += rowH + gap;
    }
}

} // namespace cadence
