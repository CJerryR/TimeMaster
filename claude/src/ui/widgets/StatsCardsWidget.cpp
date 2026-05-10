#include "StatsCardsWidget.h"
#include "../Theme.h"

#include <QPaintEvent>
#include <QPainter>
#include <QFontMetrics>

namespace cadence {

StatsCardsWidget::StatsCardsWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(120);
}

QSize StatsCardsWidget::sizeHint() const {
    return QSize(800, 120);
}

void StatsCardsWidget::setCards(const QVector<StatCard>& cards) {
    m_cards = cards;
    update();
}

void StatsCardsWidget::paintEvent(QPaintEvent*) {
    if (m_cards.isEmpty()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& th = Theme::instance();
    const int n = m_cards.size();
    const int gap = 16;
    const int totalGap = gap * (n - 1);
    const int cardW = (width() - totalGap) / n;
    const int cardH = height() - 4;

    for (int i = 0; i < n; ++i) {
        const QRect r(i * (cardW + gap), 2, cardW, cardH);

        // 卡片背景
        p.setBrush(th.bgContainer());
        p.setPen(QPen(th.borderColor(), 1));
        p.drawRoundedRect(r, 12, 12);

        const StatCard& c = m_cards[i];
        const int padX = 20;

        // 顶部行：图标 + 环比
        QFont iconFont = font();
        iconFont.setPixelSize(22);
        p.setFont(iconFont);
        p.setPen(th.textPrimary());
        p.drawText(QRect(r.x() + padX, r.y() + 16, 32, 32),
                   Qt::AlignLeft | Qt::AlignVCenter, c.icon);

        if (!c.delta.isEmpty()) {
            QFont deltaFont = font();
            deltaFont.setPixelSize(11);
            deltaFont.setWeight(QFont::DemiBold);
            p.setFont(deltaFont);
            QColor dc(c.deltaColor.isEmpty() ? "#999999" : c.deltaColor);
            p.setPen(dc);
            QFontMetrics fm(deltaFont);
            int dw = fm.horizontalAdvance(c.delta) + 12;
            int dh = 20;
            QRect dr(r.right() - padX - dw, r.y() + 18, dw, dh);
            QColor bg = dc; bg.setAlpha(28);
            p.setBrush(bg);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(dr, 10, 10);
            p.setPen(dc);
            p.drawText(dr, Qt::AlignCenter, c.delta);
        }

        // 主数值
        QFont valFont = font();
        valFont.setPixelSize(24);
        valFont.setWeight(QFont::Bold);
        p.setFont(valFont);
        p.setPen(th.textPrimary());
        p.drawText(QRect(r.x() + padX, r.y() + cardH - 56, cardW - padX * 2, 28),
                   Qt::AlignLeft | Qt::AlignVCenter, c.value);

        // 标签
        QFont labelFont = font();
        labelFont.setPixelSize(13);
        p.setFont(labelFont);
        p.setPen(th.textSecondary());
        p.drawText(QRect(r.x() + padX, r.y() + cardH - 26, cardW - padX * 2, 20),
                   Qt::AlignLeft | Qt::AlignVCenter, c.label);
    }
}

} // namespace cadence
