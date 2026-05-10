#pragma once

#include <QMainWindow>

class QStackedWidget;

namespace timeplan {

class Sidebar;
class CalendarPage;
class AnalyticsPage;
class ChatPage;
class Database;
class DeepSeekClient;

/**
 * 主窗口：左侧 Sidebar + 右侧 QStackedWidget(三个页面)
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(Database *db, DeepSeekClient *ai, QWidget *parent = nullptr);

private slots:
    void applyTheme();
    void onThemeToggle();
    void onSettings();

private:
    Database *m_db;
    DeepSeekClient *m_ai;

    Sidebar *m_sidebar;
    QStackedWidget *m_stack;
    CalendarPage *m_calendarPage;
    AnalyticsPage *m_analyticsPage;
    ChatPage *m_chatPage;
};

} // namespace timeplan
