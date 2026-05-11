#pragma once

#include <QWidget>
#include <QVector>

class QPushButton;
class QLabel;
class QVBoxLayout;

namespace timemaster {

/**
 * 左侧导航栏
 *  - 顶部 Logo
 *  - 三个导航项：日历 / 分析 / 对话
 *  - 底部：主题切换 + 设置
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
    QPushButton *makeNavButton(const QString &icon, const QString &label);

    QLabel *m_logo = nullptr;
    QVector<QPushButton*> m_navButtons;
    QPushButton *m_themeBtn = nullptr;
    QPushButton *m_settingsBtn = nullptr;
    NavItem m_current = Calendar;
};

} // namespace timemaster
