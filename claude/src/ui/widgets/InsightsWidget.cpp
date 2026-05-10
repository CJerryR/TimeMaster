#include "InsightsWidget.h"
#include "../Theme.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <algorithm>

namespace cadence {

InsightsWidget::InsightsWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(40);
}

void InsightsWidget::setInsights(const QStringList& items) {
    m_items = items;
    setMinimumHeight(computeHeight());
    updateGeometry();
    update();
}

int InsightsWidget::computeHeight() const {
    if (m_items.isEmpty()) return 0;
    QFont f = font(); f.setPixelSize(13);
    QFontMetrics fm(f);
    int total = 24 + 28; // padding + title row
    int textW = std::max(200, width() - 24 - 18);
    for (const auto& s : m_items) {
        QRect br = fm.boundingRect(QRect(0, 0, textW, 1000),
                                    Qt::TextWordWrap | Qt::AlignLeft, s);
        total += br.height() + 8;
    }
    return total + 12;
}

QSize InsightsWidget::sizeHint() const {
    return QSize(700, computeHeight());
}

void InsightsWidget::paintEvent(QPaintEvent*) {
    if (m_items.isEmpty()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const auto& th = Theme::instance();

    // 卡片背景
    p.setBrush(th.bgContainer());
    p.setPen(QPen(th.borderColor(), 1));
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 12, 12);

    const int padX = 24;
    int y = 18;

    // 标题
    QFont titleFont = font();
    titleFont.setPixelSize(15);
    titleFont.setWeight(QFont::DemiBold);
    p.setFont(titleFont);
    p.setPen(th.textPrimary());
    p.drawText(QRect(padX, y, width() - padX * 2, 22),
               Qt::AlignLeft | Qt::AlignVCenter, "💡 智能洞察");
    y += 28;

    // 列表项
    QFont itemFont = font(); itemFont.setPixelSize(13);
    p.setFont(itemFont);
    QFontMetrics fm(itemFont);
    int textX = padX + 18;
    int textW = width() - textX - padX;

    for (const auto& s : m_items) {
        QRect br = fm.boundingRect(QRect(0, 0, textW, 1000),
                                    Qt::TextWordWrap | Qt::AlignLeft, s);
        // 圆点
        p.setPen(Qt::NoPen);
        p.setBrush(th.brand());
        p.drawEllipse(QPoint(padX + 4, y + 8), 3, 3);

        p.setPen(th.textSecondary());
        p.drawText(QRect(textX, y, textW, br.height()),
                   Qt::TextWordWrap | Qt::AlignLeft, s);

        y += br.height() + 8;
    }
}

} // namespace cadence
