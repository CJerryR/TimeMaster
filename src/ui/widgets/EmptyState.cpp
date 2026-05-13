//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "EmptyState.h"
#include "../Theme.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

namespace timemaster {

// 构造函数：创建居中空状态卡片（标题+副标题+进度+操作按钮）并连接主题信号
EmptyState::EmptyState(QWidget *parent) : QWidget(parent) {
    setObjectName("EmptyState");
    setAttribute(Qt::WA_StyledBackground, false);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(40, 40, 40, 40);
    outer->setSpacing(0);
    outer->addStretch();

    auto *box = new QWidget;
    box->setObjectName("EmptyStateCard");
    box->setMinimumWidth(420);   // V4.3 #5 — 给卡片一个最小宽度，看起来稳重
    auto *boxLay = new QVBoxLayout(box);
    boxLay->setContentsMargins(44, 38, 44, 38);
    boxLay->setSpacing(12);
    boxLay->setAlignment(Qt::AlignCenter);

    m_title = new QLabel;
    m_title->setObjectName("EmptyTitle");
    m_title->setAlignment(Qt::AlignCenter);
    QFont tf;
    tf.setPointSize(18);
    tf.setWeight(QFont::DemiBold);
    m_title->setFont(tf);
    boxLay->addWidget(m_title);

    m_subtitle = new QLabel;
    m_subtitle->setObjectName("EmptySubtitle");
    m_subtitle->setAlignment(Qt::AlignCenter);
    m_subtitle->setWordWrap(true);
    boxLay->addWidget(m_subtitle);

    m_progress = new QLabel;
    m_progress->setObjectName("EmptyProgress");
    m_progress->setAlignment(Qt::AlignCenter);
    m_progress->hide();
    boxLay->addWidget(m_progress);

    boxLay->addSpacing(8);

    m_actionsRow = new QWidget;
    m_actionsLayout = new QHBoxLayout(m_actionsRow);
    m_actionsLayout->setContentsMargins(0, 0, 0, 0);
    m_actionsLayout->setSpacing(10);
    m_actionsLayout->setAlignment(Qt::AlignCenter);
    boxLay->addWidget(m_actionsRow);

    auto *centerRow = new QHBoxLayout;
    centerRow->addStretch();
    centerRow->addWidget(box);
    centerRow->addStretch();
    outer->addLayout(centerRow);
    outer->addStretch();

    connect(&Theme::instance(), &Theme::changed, this, &EmptyState::applyTheme);
    applyTheme();
}

// 设置标题文本
void EmptyState::setTitle(const QString &t)    { m_title->setText(t); }
// 设置副标题文本
void EmptyState::setSubtitle(const QString &t) { m_subtitle->setText(t); }

// 设置进度提示（非空显示，空则隐藏）
void EmptyState::setProgress(const QString &t) {
    if (t.isEmpty()) {
        m_progress->hide();
    } else {
        m_progress->setText(t);
        m_progress->show();
    }
}

// 删除所有操作按钮
void EmptyState::clearActions() {
    for (auto *b : m_actionButtons) b->deleteLater();
    m_actionButtons.clear();
}

// 添加操作按钮并连接点击回调
void EmptyState::addAction(const QString &label, const std::function<void()> &cb) {
    auto *btn = new QPushButton(label);
    btn->setObjectName("EmptyActionBtn");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(36);
    connect(btn, &QPushButton::clicked, this, [cb]{ cb(); });
    m_actionsLayout->addWidget(btn);
    m_actionButtons.append(btn);
    applyTheme();
}

// 应用 QSS：卡片背景/边框/圆角 + 按钮悬停效果
void EmptyState::applyTheme() {
    auto &t = Theme::instance();
    setStyleSheet(QString(R"(
        QWidget#EmptyState { background: transparent; }
        QWidget#EmptyStateCard {
            background-color: %4;
            border: 1px solid %5;
            border-radius: 16px;
        }
        QLabel#EmptyTitle {
            color: %1;
            background: transparent;
        }
        QLabel#EmptySubtitle {
            color: %2;
            font-size: 14px;
            background: transparent;
        }
        QLabel#EmptyProgress {
            color: %3;
            font-size: 13px;
            font-weight: 600;
            background: transparent;
            padding-top: 4px;
        }
        QPushButton#EmptyActionBtn {
            background-color: %7;
            color: %1;
            border: 1px solid %5;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 500;
            outline: 0;
        }
        QPushButton#EmptyActionBtn:hover {
            background-color: %6;
            color: %1;
            border-color: %3;
        }
    )")
    .arg(t.textPrimary().name())
    .arg(t.textSecondary().name())
    .arg(t.brand().name())
    .arg(t.cardBgRgba())      // V4.3 #5 — 不再是 transparent，给一个明确的卡片底
    .arg(t.strokeRgba())
    .arg(t.cardBgHoverRgba())
    .arg(t.bgContainer().name()));
}

} // namespace timemaster
