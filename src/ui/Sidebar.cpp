#include "Sidebar.h"
#include "Theme.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

namespace timeplan {

Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    setObjectName("Sidebar");
    setFixedWidth(240);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 20, 16, 24);
    root->setSpacing(12);

    // ---- Logo 区 ----
    auto *logoRow = new QHBoxLayout;
    logoRow->setContentsMargins(0, 0, 0, 0);
    logoRow->setSpacing(10);

    m_logo = new QLabel("时");
    m_logo->setObjectName("LogoBox");
    m_logo->setFixedSize(36, 36);
    m_logo->setAlignment(Qt::AlignCenter);

    auto *brandText = new QLabel("时智");
    brandText->setObjectName("BrandText");

    logoRow->addWidget(m_logo);
    logoRow->addWidget(brandText);
    logoRow->addStretch();
    root->addLayout(logoRow);

    auto *subTitle = new QLabel("智能日程管理");
    subTitle->setObjectName("SidebarSubtitle");
    subTitle->setWordWrap(true);
    root->addWidget(subTitle);

    root->addSpacing(16);

    // ---- 导航项 ----
    m_navButtons << makeNavButton("📅", "日历")
                 << makeNavButton("📊", "分析")
                 << makeNavButton("💬", "AI 对话");

    for (int i = 0; i < m_navButtons.size(); ++i) {
        QPushButton *b = m_navButtons[i];
        root->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, i] { onNavClicked(i); });
    }

    root->addStretch();

    // ---- 底部工具区 ----
    auto *divider = new QFrame;
    divider->setObjectName("SidebarDivider");
    divider->setFixedHeight(1);
    root->addWidget(divider);

    auto *toolRow = new QHBoxLayout;
    toolRow->setContentsMargins(0, 8, 0, 0);
    toolRow->setSpacing(8);

    m_themeBtn = new QPushButton("🌙");
    m_themeBtn->setObjectName("SidebarToolBtn");
    m_themeBtn->setFixedSize(44, 44);
    m_themeBtn->setToolTip("切换主题");
    connect(m_themeBtn, &QPushButton::clicked, this, &Sidebar::themeToggleRequested);

    m_settingsBtn = new QPushButton("⚙");
    m_settingsBtn->setObjectName("SidebarToolBtn");
    m_settingsBtn->setFixedSize(44, 44);
    m_settingsBtn->setToolTip("设置");
    connect(m_settingsBtn, &QPushButton::clicked, this, &Sidebar::settingsRequested);

    toolRow->addWidget(m_themeBtn);
    toolRow->addWidget(m_settingsBtn);
    toolRow->addStretch();
    root->addLayout(toolRow);

    setCurrentItem(Calendar);

    connect(&Theme::instance(), &Theme::changed, this, &Sidebar::applyTheme);
    applyTheme();
}

QPushButton *Sidebar::makeNavButton(const QString &icon, const QString &label) {
    auto *btn = new QPushButton(QString("  %1   %2").arg(icon, label));
    btn->setObjectName("SidebarNavBtn");
    btn->setMinimumHeight(40);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(true);
    return btn;
}

void Sidebar::setCurrentItem(NavItem item) {
    m_current = item;
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setChecked(i == int(item));
    }
    updateButtonStyles();
}

void Sidebar::onNavClicked(int idx) {
    setCurrentItem(static_cast<NavItem>(idx));
    emit navigated(idx);
}

void Sidebar::updateButtonStyles() {
    // QSS 中 :checked 选择器自动生效，无需手动刷新
}

void Sidebar::applyTheme() {
    Theme &t = Theme::instance();
    QString brand = t.brand().name();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();
    QString hover = t.bgHover().name();
    QString brandLight = QString("rgba(%1, %2, %3, 30)")
                            .arg(t.brand().red()).arg(t.brand().green()).arg(t.brand().blue());
    QString divider = t.stroke().name();
    QString bgContainer = t.bgContainer().name();

    setStyleSheet(QString(R"(
        QWidget#Sidebar {
            background-color: %1;
            border-right: 1px solid %2;
        }
        QLabel#LogoBox {
            background-color: %3;
            color: white;
            border-radius: 8px;
            font-size: 18px;
            font-weight: 600;
        }
        QLabel#BrandText {
            color: %4;
            font-size: 16px;
            font-weight: 600;
        }
        QLabel#SidebarSubtitle {
            color: %5;
            font-size: 11px;
            margin-left: 46px;
            margin-top: 0px;
        }
        QPushButton#SidebarNavBtn {
            text-align: left;
            padding: 8px 12px;
            border: none;
            border-radius: 8px;
            background-color: transparent;
            color: %5;
            font-size: 14px;
        }
        QPushButton#SidebarNavBtn:hover {
            background-color: %6;
            color: %4;
        }
        QPushButton#SidebarNavBtn:checked {
            background-color: %7;
            color: %3;
            font-weight: 600;
        }
        QFrame#SidebarDivider {
            background-color: %2;
            border: none;
        }
        QPushButton#SidebarToolBtn {
            border: none;
            border-radius: 8px;
            background-color: transparent;
            color: %5;
            font-size: 22px;
        }
        QPushButton#SidebarToolBtn:hover {
            background-color: %6;
            color: %4;
        }
    )")
        .arg(bgContainer, divider, brand, textPrim, textSec, hover, brandLight));

    // 主题图标随模式更新
    m_themeBtn->setText(t.mode() == Theme::Dark ? "☀" : "🌙");
}

} // namespace timeplan
