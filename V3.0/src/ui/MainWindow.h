#pragma once

#include <QMainWindow>

class QStackedWidget;

namespace timemaster {

class Sidebar;
class CalendarPage;
class AnalyticsPage;
class ChatPage;
class Database;
class DeepSeekClient;
class PageBackground;

/**
 * 主窗口：左侧 Sidebar + 右侧 QStackedWidget(三个页面)
 *
 * 使用 PageBackground widget 在最底层绘制对角渐变，
 * 上层的卡片/侧边栏均使用 rgba 半透明背景，让渐变透出形成层次感。
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

    PageBackground *m_bg;
    Sidebar *m_sidebar;
    QStackedWidget *m_stack;
    CalendarPage *m_calendarPage;
    AnalyticsPage *m_analyticsPage;
    ChatPage *m_chatPage;
};

} // namespace timemaster
