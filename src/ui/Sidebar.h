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
    void languageToggleRequested();

private slots:
    void applyTheme();
    void applyLanguage();
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
