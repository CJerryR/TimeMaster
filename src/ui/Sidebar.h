#pragma once

#include <QWidget>
#include <QVector>

class QPushButton;
class QLabel;
class QVBoxLayout;

namespace timemaster {

class SidebarLogo;

/**
 * 左侧导航栏
 *  - 顶部 Logo：自绘贴图（用当前选中的 App 图标）
 *  - 三个导航项：日历 / 分析 / 对话（QPainter 贴图图标，跟主题变色）
 *  - 底部：主题切换 + 设置（贴图按钮，确保不被裁切）
 *  - 半透明背景，让主窗口的渐变隐隐透出
 */
class Sidebar : public QWidget {
    Q_OBJECT
public:
    enum NavItem { Calendar = 0, Analytics = 1, Chat = 2 };

    explicit Sidebar(QWidget *parent = nullptr);

    void setCurrentItem(NavItem item);
    NavItem currentItem() const { return m_current; }

signals:
    void navigated(int index);
    void themeToggleRequested();
    void settingsRequested();

private slots:
    void applyTheme();
    void onNavClicked(int idx);

private:
    QPushButton *makeNavButton(int iconId, const QString &label);
    void refreshNavIcons();

    SidebarLogo *m_logoWidget = nullptr;
    QVector<QPushButton*> m_navButtons;
    QPushButton *m_themeBtn = nullptr;
    QPushButton *m_settingsBtn = nullptr;
    NavItem m_current = Calendar;
};

} // namespace timemaster
