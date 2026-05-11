#include "SourceDistributionWidget.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>

namespace timemaster {

static const QColor C_MANUAL  = QColor("#3B82F6"); // 蓝
static const QColor C_AIPARSE = QColor("#8B5CF6"); // 紫
static const QColor C_CHAT    = QColor("#F59E0B"); // 橙

SourceDistributionWidget::SourceDistributionWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
    setMinimumHeight(200);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(2);

    m_title = new QLabel("📥  数据来源");
    QFont tf;
    tf.setPointSize(11);
    tf.setWeight(QFont::DemiBold);
    m_title->setFont(tf);
    m_title->setStyleSheet("background:transparent;");
    lay->addWidget(m_title);

    m_subtitle = new QLabel("手动 / AI 解析 / AI 对话");
    QFont sf;
    sf.setPointSize(9);
    m_subtitle->setFont(sf);
    m_subtitle->setStyleSheet("background:transparent;");
    lay->addWidget(m_subtitle);

    lay->addStretch(1);

    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, [this]() { applyTheme(); update(); });
}

void SourceDistributionWidget::setSources(int manualCount, int aiParseCount, int chatCount) {
    m_manual = std::max(0, manualCount);
    m_aiParse = std::max(0, aiParseCount);
    m_chat = std::max(0, chatCount);
    int total = m_manual + m_aiParse + m_chat;
    if (total <= 0) {
        m_subtitle->setText("暂无数据");
    } else {
        m_subtitle->setText(QString("共 %1 个事件").arg(total));
    }
    update();
}

void SourceDistributionWidget::applyTheme() {
    auto &t = Theme::instance();
    m_title->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPrimary().name()));
    m_subtitle->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textSecondary().name()));
}

void SourceDistributionWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();

    const int topReserve = 50;
    const int padL = 4;
    const int padR = 4;

    int total = m_manual + m_aiParse + m_chat;
    if (total <= 0) {
        p.setPen(theme.textPlaceholder());
        QRect r(0, topReserve, width(), height() - topReserve);
        p.drawText(r, Qt::AlignCenter, "暂无数据");
        return;
    }

    // 堆叠条
    const int barY = topReserve + 6;
    const int barH = 18;
    const int barW = width() - padL - padR;
    QRect track(padL, barY, barW, barH);
    QPainterPath trackPath;
    trackPath.addRoundedRect(track, 9, 9);
    p.fillPath(trackPath, theme.bgHover());

    double r1 = double(m_manual) / total;
    double r2 = double(m_aiParse) / total;
    double r3 = double(m_chat) / total;
    int w1 = int(barW * r1);
    int w2 = int(barW * r2);
    int w3 = barW - w1 - w2; // 防止累积舍入

    p.setClipPath(trackPath);
    p.fillRect(QRect(padL, barY, w1, barH), C_MANUAL);
    p.fillRect(QRect(padL + w1, barY, w2, barH), C_AIPARSE);
    p.fillRect(QRect(padL + w1 + w2, barY, w3, barH), C_CHAT);
    p.setClipping(false);

    // legend
    struct Item { QColor c; QString name; int count; };
    QList<Item> items = {
        { C_MANUAL,  "手动",    m_manual  },
        { C_AIPARSE, "AI 解析", m_aiParse },
        { C_CHAT,    "AI 对话", m_chat    },
    };

    int legendY = barY + barH + 18;
    QFont labelFont = font();
    labelFont.setPointSize(10);
    p.setFont(labelFont);

    for (const auto &it : items) {
        // 色块
        QRect dot(padL, legendY + 4, 12, 12);
        QPainterPath dotPath;
        dotPath.addRoundedRect(dot, 3, 3);
        p.fillPath(dotPath, it.c);

        // 名称
        p.setPen(theme.textPrimary());
        QRect nameRect(padL + 18, legendY, 90, 20);
        p.drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, it.name);

        // 数量 + 百分比
        double pct = total > 0 ? double(it.count) / total * 100.0 : 0.0;
        QString right = QString("%1 个 · %2%").arg(it.count).arg(QString::number(pct, 'f', 0));
        p.setPen(theme.textSecondary());
        QRect rRect(width() - padR - 140, legendY, 140, 20);
        p.drawText(rRect, Qt::AlignVCenter | Qt::AlignRight, right);

        legendY += 22;
    }
}

} // namespace timemaster
