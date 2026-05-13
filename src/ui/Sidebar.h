//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include <QVector>

class QPushButton;
class QLabel;
class QVBoxLayout;

namespace timemaster {

class SidebarLogo;

/**
 * V4 sidebar: 180 px wide, slim ghost tool buttons, no English subtitle.
 */
// 左侧导航栏（180px）：Logo、日历/分析/聊天三个导航按钮、底部语言切换/主题切换/设置
class Sidebar : public QWidget {
    Q_OBJECT
public:
    enum NavItem { Calendar = 0, Analytics = 1, Chat = 2 };

    // 构造函数：创建导航按钮、底部工具栏，连接信号
    explicit Sidebar(QWidget *parent = nullptr);

    // 设置当前选中导航项并刷新图标
    void setCurrentItem(NavItem item);
    // 返回当前选中导航项
    NavItem currentItem() const { return m_current; }

signals:
    // 导航按钮点击信号
    void navigated(int index);
    // 主题切换请求信号
    void themeToggleRequested();
    // 设置请求信号
    void settingsRequested();
    // 语言切换请求信号
    void languageToggleRequested();

private slots:
    // 应用主题样式
    void applyTheme();
    // 应用语言切换
    void applyLanguage();
    // 处理导航按钮点击
    void onNavClicked(int idx);

private:
    QPushButton *makeNavButton(int iconId, const QString &i18nKey);
    void refreshNavIcons();

    SidebarLogo *m_logoWidget = nullptr;
    QLabel *m_brandText = nullptr;
    QVector<QPushButton*> m_navButtons;
    QVector<QString>      m_navKeys;
    QPushButton *m_themeBtn = nullptr;
    QPushButton *m_settingsBtn = nullptr;
    QPushButton *m_langBtn = nullptr;
    NavItem m_current = Calendar;
};

} // namespace timemaster
