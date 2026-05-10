#include "MainWindow.h"
#include "Sidebar.h"
#include "CalendarPage.h"
#include "AnalyticsPage.h"
#include "ChatPage.h"
#include "SettingsDialog.h"
#include "Theme.h"
#include "../core/Database.h"
#include "../core/DeepSeekClient.h"

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QWidget>

namespace timeplan {

MainWindow::MainWindow(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QMainWindow(parent), m_db(db), m_ai(ai)
{
    setWindowTitle("时智 — 智能日程管理");
    resize(1280, 820);
    setMinimumSize(960, 640);

    auto *central = new QWidget;
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_sidebar = new Sidebar;
    root->addWidget(m_sidebar);

    m_stack = new QStackedWidget;
    m_calendarPage = new CalendarPage(m_db, m_ai);
    m_analyticsPage = new AnalyticsPage(m_db);
    m_chatPage = new ChatPage(m_db, m_ai);

    m_stack->addWidget(m_calendarPage);    // 0
    m_stack->addWidget(m_analyticsPage);   // 1
    m_stack->addWidget(m_chatPage);        // 2
    root->addWidget(m_stack, 1);

    setCentralWidget(central);

    connect(m_sidebar, &Sidebar::navigated, m_stack, &QStackedWidget::setCurrentIndex);
    connect(m_sidebar, &Sidebar::themeToggleRequested, this, &MainWindow::onThemeToggle);
    connect(m_sidebar, &Sidebar::settingsRequested, this, &MainWindow::onSettings);

    connect(&Theme::instance(), &Theme::changed, this, &MainWindow::applyTheme);
    applyTheme();
}

void MainWindow::onThemeToggle() {
    Theme::instance().toggle();
}

void MainWindow::onSettings() {
    SettingsDialog dlg(m_ai, m_db->filePath(), this);
    dlg.exec();
}

void MainWindow::applyTheme() {
    setStyleSheet(Theme::instance().globalStylesheet());
}

} // namespace timeplan
