//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "Sidebar.h"
#include "Theme.h"
#include "IconRenderer.h"
#include "../core/I18n.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QPaintEvent>

namespace timemaster {

// Logo 方块 32x32 (was 40x40)
class SidebarLogo : public QWidget {
public:
    // 构造函数：设置固定尺寸
    explicit SidebarLogo(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(32, 32);
    }
protected:
    // 绘制应用图标
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        auto pm = IconRenderer::appIcon(IconRenderer::defaultAppIcon(), 32);
        p.drawPixmap(0, 0, pm);
    }
};

// 构造函数：布局 Logo、导航按钮、底部工具栏
Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    setObjectName("Sidebar");
    setFixedWidth(180);   // V4 § 6.1: 232 -> 180

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 18, 14, 18);   // V4 § 6.1
    root->setSpacing(4);

    // ---- Logo + brand text ----
    auto *logoRow = new QHBoxLayout;
    logoRow->setContentsMargins(2, 0, 0, 0);
    logoRow->setSpacing(10);

    m_logoWidget = new SidebarLogo;
    logoRow->addWidget(m_logoWidget);

    m_brandText = new QLabel;
    m_brandText->setObjectName("BrandText");
    logoRow->addWidget(m_brandText);
    logoRow->addStretch();
    root->addLayout(logoRow);

    root->addSpacing(22);

    // ---- Nav items ----
    m_navKeys = {QStringLiteral("nav.calendar"),
                 QStringLiteral("nav.analytics"),
                 QStringLiteral("nav.chat")};
    m_navButtons << makeNavButton(IconRenderer::NavCalendar,  m_navKeys[0])
                 << makeNavButton(IconRenderer::NavAnalytics, m_navKeys[1])
                 << makeNavButton(IconRenderer::NavChat,      m_navKeys[2]);

    for (int i = 0; i < m_navButtons.size(); ++i) {
        QPushButton *b = m_navButtons[i];
        root->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, i] { onNavClicked(i); });
    }

    root->addStretch();

    auto *divider = new QFrame;
    divider->setObjectName("SidebarDivider");
    divider->setFixedHeight(1);
    root->addWidget(divider);
    root->addSpacing(8);

    // ---- Bottom ghost tools (28x28) ----
    auto *toolRow = new QHBoxLayout;
    toolRow->setContentsMargins(0, 0, 0, 0);
    toolRow->setSpacing(4);

    m_langBtn = new QPushButton;
    m_langBtn->setObjectName("SidebarToolBtn");
    m_langBtn->setFixedSize(28, 28);
    m_langBtn->setIconSize(QSize(16, 16));
    m_langBtn->setCursor(Qt::PointingHandCursor);
    m_langBtn->setFocusPolicy(Qt::NoFocus);  // V4.2 #3
    connect(m_langBtn, &QPushButton::clicked, this, &Sidebar::languageToggleRequested);

    m_themeBtn = new QPushButton;
    m_themeBtn->setObjectName("SidebarToolBtn");
    m_themeBtn->setFixedSize(28, 28);
    m_themeBtn->setIconSize(QSize(16, 16));
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    m_themeBtn->setFocusPolicy(Qt::NoFocus);  // V4.2 #3
    connect(m_themeBtn, &QPushButton::clicked, this, &Sidebar::themeToggleRequested);

    m_settingsBtn = new QPushButton;
    m_settingsBtn->setObjectName("SidebarToolBtn");
    m_settingsBtn->setFixedSize(28, 28);
    m_settingsBtn->setIconSize(QSize(16, 16));
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    m_settingsBtn->setFocusPolicy(Qt::NoFocus);  // V4.2 #3
    connect(m_settingsBtn, &QPushButton::clicked, this, &Sidebar::settingsRequested);

    toolRow->addWidget(m_langBtn);
    toolRow->addWidget(m_themeBtn);
    toolRow->addWidget(m_settingsBtn);
    toolRow->addStretch();
    root->addLayout(toolRow);

    setCurrentItem(Calendar);

    connect(&Theme::instance(), &Theme::changed, this, &Sidebar::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, &Sidebar::applyLanguage);
    applyLanguage();
    applyTheme();
}

// 创建导航按钮
QPushButton *Sidebar::makeNavButton(int iconId, const QString &i18nKey) {
    auto *btn = new QPushButton(I18n::t(i18nKey));
    btn->setObjectName("SidebarNavBtn");
    btn->setMinimumHeight(44);    // V4.2 #4: taller for bigger text
    btn->setIconSize(QSize(18, 18));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(true);
    // V4.2 #3: kill the dashed focus rectangle by not accepting keyboard focus.
    // We still rely on global outline:0 in QSS, but this is belt-and-suspenders.
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setProperty("iconId", iconId);
    btn->setProperty("i18nKey", i18nKey);
    return btn;
}

// 设置当前选中项，更新选中状态和图标
void Sidebar::setCurrentItem(NavItem item) {
    m_current = item;
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setChecked(i == int(item));
    }
    refreshNavIcons();
}

// 处理导航按钮点击，发射 navigated 信号
void Sidebar::onNavClicked(int idx) {
    setCurrentItem(static_cast<NavItem>(idx));
    emit navigated(idx);
}

// 刷新所有导航图标颜色
void Sidebar::refreshNavIcons() {
    Theme &t = Theme::instance();
    for (int i = 0; i < m_navButtons.size(); ++i) {
        auto *b = m_navButtons[i];
        bool active = (i == int(m_current));
        QColor c = active ? t.brand() : t.textSecondary();
        int id = b->property("iconId").toInt();
        b->setIcon(IconRenderer::icon(static_cast<IconRenderer::Icon>(id), c, 18));
    }
}

// 应用语言：更新品牌文字、导航文字、工具提示
void Sidebar::applyLanguage() {
    if (m_brandText) m_brandText->setText(I18n::t("sidebar.brand"));
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setText(I18n::t(m_navKeys[i]));
    }
    if (m_themeBtn)    m_themeBtn->setToolTip(I18n::t("sidebar.theme_tip"));
    if (m_settingsBtn) m_settingsBtn->setToolTip(I18n::t("sidebar.settings_tip"));
    if (m_langBtn) {
        m_langBtn->setToolTip(I18n::t("sidebar.lang_tip"));
        // V4.3.3 #2 — 语言切换按钮上的「A」/「中」字形必须随语言切换。
        // 原本只在 applyTheme() 里更新，这导致用户点按钮后只看到导航栏文字
        // 切了，按钮自己的字形要等下一次切主题才会更新。改成 applyLanguage()
        // 里同步刷新，applyTheme() 里也保留（兜底首次进入）。
        m_langBtn->setText(I18n::instance().isEnglish() ? "中" : "A");
    }
}

// 应用主题：更新侧边栏样式、图标颜色
void Sidebar::applyTheme() {
    Theme &t = Theme::instance();
    QString brand = t.brand().name();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();

    QString brand14 = QString("rgba(%1,%2,%3,0.14)")
        .arg(t.brand().red()).arg(t.brand().green()).arg(t.brand().blue());

    setStyleSheet(QString(R"(
        QWidget#Sidebar {
            background-color: %1;
            border-right: 1px solid %2;
        }
        QLabel#BrandText {
            color: %4;
            font-size: 15px;
            font-weight: 600;
            letter-spacing: 0.2px;
        }
        QPushButton#SidebarNavBtn {
            text-align: left;
            padding: 8px 10px 8px 14px;
            border: none;
            border-radius: 8px;
            background-color: transparent;
            color: %5;
            font-size: 15px;
            font-weight: 500;
            outline: 0;
        }
        QPushButton#SidebarNavBtn:hover {
            background-color: rgba(120,120,140,0.08);
            color: %4;
        }
        QPushButton#SidebarNavBtn:checked {
            background-color: %8;
            color: %3;
            font-weight: 600;
        }
        QFrame#SidebarDivider {
            background-color: %2;
            border: none;
        }
        QPushButton#SidebarToolBtn {
            border: 1px solid transparent;
            border-radius: 6px;
            background-color: transparent;
            color: %5;
            padding: 0;
        }
        QPushButton#SidebarToolBtn:hover {
            background-color: %7;
            border-color: %2;
            color: %4;
        }
    )")
    /*1*/.arg(t.sidebarBgRgba())
    /*2*/.arg(t.strokeRgba())
    /*3*/.arg(brand)
    /*4*/.arg(textPrim)
    /*5*/.arg(textSec)
    /*6*/.arg(t.textPlaceholder().name())
    /*7*/.arg(t.cardBgHoverRgba())
    /*8*/.arg(brand14));

    if (m_themeBtn) m_themeBtn->setIcon(IconRenderer::icon(
        t.mode() == Theme::Dark ? IconRenderer::ThemeSun : IconRenderer::ThemeMoon,
        textPrim, 16));
    if (m_settingsBtn) m_settingsBtn->setIcon(IconRenderer::icon(
        IconRenderer::Settings, textPrim, 16));
    if (m_langBtn) {
        // 用普通文字 "A/中" 作为切换语言的标记（IconRenderer 没有专门图标）
        m_langBtn->setText(I18n::instance().isEnglish() ? "中" : "A");
        m_langBtn->setStyleSheet(QString(
            "QPushButton#SidebarToolBtn { color:%1; font-weight:600; font-size:12px; }")
            .arg(textPrim));
    }
    refreshNavIcons();
    if (m_logoWidget) m_logoWidget->update();
}

} // namespace timemaster
