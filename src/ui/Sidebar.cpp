#include "Sidebar.h"
#include "Theme.h"
#include "IconRenderer.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QPaintEvent>

namespace timemaster {

// 自绘的左上角 Logo 方块（贴图风，含主图标 + 主题色背景圆角矩形）
class SidebarLogo : public QWidget {
public:
    explicit SidebarLogo(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(40, 40);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        // 用当前选中的 App 图标作为 logo（已含背景色块）
        auto pm = IconRenderer::appIcon(IconRenderer::defaultAppIcon(), 40);
        p.drawPixmap(0, 0, pm);
    }
};

Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    setObjectName("Sidebar");
    setFixedWidth(232);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 22, 18, 22);
    root->setSpacing(6);

    // ---- Logo 区 ----
    auto *logoRow = new QHBoxLayout;
    logoRow->setContentsMargins(2, 0, 0, 0);
    logoRow->setSpacing(11);

    m_logoWidget = new SidebarLogo;
    logoRow->addWidget(m_logoWidget);

    auto *brandWrap = new QVBoxLayout;
    brandWrap->setContentsMargins(0, 0, 0, 0);
    brandWrap->setSpacing(0);
    auto *brandText = new QLabel("时间管理大师");
    brandText->setObjectName("BrandText");
    auto *subText = new QLabel("Time Master");
    subText->setObjectName("BrandSub");
    brandWrap->addWidget(brandText);
    brandWrap->addWidget(subText);

    logoRow->addLayout(brandWrap);
    logoRow->addStretch();
    root->addLayout(logoRow);

    root->addSpacing(26);

    // ---- 导航项 ----
    m_navButtons << makeNavButton(IconRenderer::NavCalendar,  "日历")
                 << makeNavButton(IconRenderer::NavAnalytics, "分析")
                 << makeNavButton(IconRenderer::NavChat,      "AI 对话");

    for (int i = 0; i < m_navButtons.size(); ++i) {
        QPushButton *b = m_navButtons[i];
        root->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, i] { onNavClicked(i); });
    }

    root->addStretch();

    // ---- 底部分割线 ----
    auto *divider = new QFrame;
    divider->setObjectName("SidebarDivider");
    divider->setFixedHeight(1);
    root->addWidget(divider);
    root->addSpacing(10);

    // ---- 底部工具按钮 ----
    auto *toolRow = new QHBoxLayout;
    toolRow->setContentsMargins(0, 0, 0, 0);
    toolRow->setSpacing(10);

    m_themeBtn = new QPushButton;
    m_themeBtn->setObjectName("SidebarToolBtn");
    m_themeBtn->setFixedSize(44, 44);
    m_themeBtn->setIconSize(QSize(22, 22));
    m_themeBtn->setToolTip("切换浅色 / 深色主题");
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    // 关键：确保不会被相邻 widget 裁掉 —— 给一点 margin 留白
    connect(m_themeBtn, &QPushButton::clicked, this, &Sidebar::themeToggleRequested);

    m_settingsBtn = new QPushButton;
    m_settingsBtn->setObjectName("SidebarToolBtn");
    m_settingsBtn->setFixedSize(44, 44);
    m_settingsBtn->setIconSize(QSize(22, 22));
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

QPushButton *Sidebar::makeNavButton(int iconId, const QString &label) {
    auto *btn = new QPushButton(label);
    btn->setObjectName("SidebarNavBtn");
    btn->setMinimumHeight(44);
    btn->setIconSize(QSize(20, 20));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(true);
    btn->setProperty("iconId", iconId);
    return btn;
}

void Sidebar::setCurrentItem(NavItem item) {
    m_current = item;
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setChecked(i == int(item));
    }
    // 刷新图标颜色（选中态用 brand 色）
    refreshNavIcons();
}

void Sidebar::onNavClicked(int idx) {
    setCurrentItem(static_cast<NavItem>(idx));
    emit navigated(idx);
}

void Sidebar::refreshNavIcons() {
    Theme &t = Theme::instance();
    for (int i = 0; i < m_navButtons.size(); ++i) {
        auto *b = m_navButtons[i];
        bool active = (i == int(m_current));
        QColor c = active ? t.brand() : t.textSecondary();
        int id = b->property("iconId").toInt();
        b->setIcon(IconRenderer::icon(static_cast<IconRenderer::Icon>(id), c, 20));
    }
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
            padding: 9px 10px 9px 16px;
            border: none;
            border-radius: 11px;
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
            background-color: rgba(217,119,87,0.14);
            color: %3;
            font-weight: 600;
        }
        QFrame#SidebarDivider {
            background-color: %2;
            border: none;
        }
        QPushButton#SidebarToolBtn {
            border: 1px solid %2;
            border-radius: 11px;
            background-color: %7;
            color: %5;
            padding: 0;
        }
        QPushButton#SidebarToolBtn:hover {
            background-color: %8;
            color: %4;
            border-color: %5;
        }
    )")
    /*1*/.arg(t.sidebarBgRgba())
    /*2*/.arg(t.strokeRgba())
    /*3*/.arg(brand)
    /*4*/.arg(textPrim)
    /*5*/.arg(textSec)
    /*6*/.arg(placeholder)
    /*7*/.arg(t.bgContainer().name())
    /*8*/.arg(t.cardBgHoverRgba()));

    // 工具按钮图标随主题切换
    m_themeBtn->setIcon(IconRenderer::icon(
        t.mode() == Theme::Dark ? IconRenderer::ThemeSun : IconRenderer::ThemeMoon,
        textPrim, 22));
    m_settingsBtn->setIcon(IconRenderer::icon(IconRenderer::Settings, textPrim, 22));
    refreshNavIcons();
    if (m_logoWidget) m_logoWidget->update();
}

} // namespace timemaster
