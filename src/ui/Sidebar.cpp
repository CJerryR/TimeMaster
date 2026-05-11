#include "Sidebar.h"
#include "Theme.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

namespace timemaster {

Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    setObjectName("Sidebar");
    setFixedWidth(232);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 24, 18, 24);
    root->setSpacing(6);

    // ---- Logo 区 ----
    auto *logoRow = new QHBoxLayout;
    logoRow->setContentsMargins(2, 0, 0, 0);
    logoRow->setSpacing(11);

    m_logo = new QLabel("时");
    m_logo->setObjectName("LogoBox");
    m_logo->setFixedSize(38, 38);
    m_logo->setAlignment(Qt::AlignCenter);

    auto *brandWrap = new QVBoxLayout;
    brandWrap->setContentsMargins(0, 0, 0, 0);
    brandWrap->setSpacing(0);
    auto *brandText = new QLabel("时间管理大师");
    brandText->setObjectName("BrandText");
    auto *subText = new QLabel("Time Master");
    subText->setObjectName("BrandSub");
    brandWrap->addWidget(brandText);
    brandWrap->addWidget(subText);

    logoRow->addWidget(m_logo);
    logoRow->addLayout(brandWrap);
    logoRow->addStretch();
    root->addLayout(logoRow);

    root->addSpacing(28);

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

    // ---- 底部 ----
    auto *divider = new QFrame;
    divider->setObjectName("SidebarDivider");
    divider->setFixedHeight(1);
    root->addWidget(divider);
    root->addSpacing(10);

    auto *toolRow = new QHBoxLayout;
    toolRow->setContentsMargins(0, 0, 0, 0);
    toolRow->setSpacing(8);

    m_themeBtn = new QPushButton("🌙");
    m_themeBtn->setObjectName("SidebarToolBtn");
    m_themeBtn->setFixedSize(42, 42);
    m_themeBtn->setToolTip("切换主题");
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_themeBtn, &QPushButton::clicked, this, &Sidebar::themeToggleRequested);

    m_settingsBtn = new QPushButton("⚙");
    m_settingsBtn->setObjectName("SidebarToolBtn");
    m_settingsBtn->setFixedSize(42, 42);
    m_settingsBtn->setToolTip("设置");
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
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
    btn->setMinimumHeight(42);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(true);
    return btn;
}

void Sidebar::setCurrentItem(NavItem item) {
    m_current = item;
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setChecked(i == int(item));
    }
}

void Sidebar::onNavClicked(int idx) {
    setCurrentItem(static_cast<NavItem>(idx));
    emit navigated(idx);
}

void Sidebar::applyTheme() {
    Theme &t = Theme::instance();
    QString brand = t.brand().name();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();
    QString placeholder = t.textPlaceholder().name();

    setStyleSheet(QString(R"(
        QWidget#Sidebar {
            background-color: %1;
            border-right: 1px solid %2;
        }
        QLabel#LogoBox {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                        stop:0 %3, stop:1 #B85A3D);
            color: white;
            border-radius: 11px;
            font-size: 19px;
            font-weight: 700;
        }
        QLabel#BrandText {
            color: %4;
            font-size: 15px;
            font-weight: 700;
            letter-spacing: 0.5px;
        }
        QLabel#BrandSub {
            color: %6;
            font-size: 10px;
            letter-spacing: 1px;
            margin-top: 1px;
        }
        QPushButton#SidebarNavBtn {
            text-align: left;
            padding: 9px 14px;
            border: none;
            border-radius: 10px;
            background-color: transparent;
            color: %5;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton#SidebarNavBtn:hover {
            background-color: rgba(120,120,140,0.08);
            color: %4;
        }
        QPushButton#SidebarNavBtn:checked {
            background-color: rgba(217,119,87,0.13);
            color: %3;
            font-weight: 600;
        }
        QFrame#SidebarDivider {
            background-color: %2;
            border: none;
        }
        QPushButton#SidebarToolBtn {
            border: 1px solid %2;
            border-radius: 10px;
            background-color: transparent;
            color: %5;
            font-size: 18px;
        }
        QPushButton#SidebarToolBtn:hover {
            background-color: rgba(120,120,140,0.10);
            color: %4;
            border-color: %5;
        }
    )")
    /*1*/.arg(t.sidebarBgRgba())
    /*2*/.arg(t.strokeRgba())
    /*3*/.arg(brand)
    /*4*/.arg(textPrim)
    /*5*/.arg(textSec)
    /*6*/.arg(placeholder));

    m_themeBtn->setText(t.mode() == Theme::Dark ? "☀" : "🌙");
}

} // namespace timemaster
