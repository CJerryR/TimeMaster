//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

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
// 主窗口：左侧 Sidebar + 右侧 QStackedWidget（日历/分析/对话三页），底部 PageBackground 绘制页面底色
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // 构造函数：创建三页面、Sidebar、导航控制、引导页延迟弹出
    explicit MainWindow(Database *db, DeepSeekClient *ai, QWidget *parent = nullptr);

private slots:
    // 应用全局主题样式表并重绘背景
    void applyTheme();
    // 切换亮/暗主题
    void onThemeToggle();
    // 打开设置对话框
    void onSettings();
    // 首次启动时弹出引导页
    void showOnboardingIfFirstRun();

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
