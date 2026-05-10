#include "SourceDistributionWidget.h"
#include "../Theme.h"

#include <QPainter>
#include <QPaintEvent>
#include <cmath>

namespace cadence {

SourceDistributionWidget::SourceDistributionWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(140);
}

QSize SourceDistributionWidget::sizeHint() const {
    return QSize(700, 140);
}

void SourceDistributionWidget::setData(int manualCount, int aiCount) {
    m_manual = manualCount;
    m_ai = aiCount;
    update();
}

void SourceDistributionWidget::paintEvent(QPaintEvent*) {
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
    QFont titleFont = font(); titleFont.setPixelSize(15); titleFont.setWeight(QFont::DemiBold);
    p.setFont(titleFont);
    p.setPen(th.textPrimary());
    p.drawText(QRect(padX, y, width() - padX * 2, 22),
               Qt::AlignLeft | Qt::AlignVCenter, "日程录入方式");
    y += 30;

    // 手动数值
    QFont bigFont = font(); bigFont.setPixelSize(32); bigFont.setWeight(QFont::Bold);
    QFont smallFont = font(); smallFont.setPixelSize(13);

    p.setFont(bigFont);
    p.setPen(th.brand());
    p.drawText(QRect(padX, y, 90, 36),
               Qt::AlignLeft | Qt::AlignVCenter, QString::number(m_manual));
    p.setFont(smallFont);
    p.setPen(th.textPlaceholder());
    p.drawText(QRect(padX, y + 36, 90, 18),
               Qt::AlignLeft | Qt::AlignVCenter, "手动录入");

    // AI 数值
    const QColor aiColor("#8b5cf6");
    p.setFont(bigFont);
    p.setPen(aiColor);
    p.drawText(QRect(padX + 130, y, 90, 36),
               Qt::AlignLeft | Qt::AlignVCenter, QString::number(m_ai));
    p.setFont(smallFont);
    p.setPen(th.textPlaceholder());
    p.drawText(QRect(padX + 130, y + 36, 90, 18),
               Qt::AlignLeft | Qt::AlignVCenter, "AI 解析");

    // 右侧 AI 占比进度条
    int total = m_manual + m_ai;
    int aiPct = total > 0 ? (int)std::round(100.0 * m_ai / total) : 0;
    const int barW = 160, barH = 10;
    const int barX = width() - padX - barW;
    const int barY = y + 18;

    p.setPen(Qt::NoPen);
    p.setBrush(th.bgComponent());
    p.drawRoundedRect(QRect(barX, barY, barW, barH), barH / 2, barH / 2);
    if (total > 0 && m_ai > 0) {
        int w = (int)(barW * (double)m_ai / total);
        p.setBrush(aiColor);
        p.drawRoundedRect(QRect(barX, barY, w, barH), barH / 2, barH / 2);
    }
    p.setFont(smallFont);
    p.setPen(th.textPlaceholder());
    p.drawText(QRect(barX, barY + barH + 4, barW, 16),
               Qt::AlignRight | Qt::AlignVCenter, QString("AI 占比 %1%").arg(aiPct));
}

} // namespace cadence
