#include "MainWindow.h"
#include "Sidebar.h"
#include "CalendarPage.h"
#include "AnalyticsPage.h"
#include "ChatPage.h"
#include "SettingsDialog.h"
#include "OnboardingDialog.h"
#include "Theme.h"
#include "../core/Database.h"
#include "../core/DeepSeekClient.h"
#include "../core/I18n.h"

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QSettings>
#include <QTimer>

namespace timemaster {

/**
 * V4 § 2.3 / § 7.4 — gradient and radial glow removed.
 * Page background is a flat bgPage() fill; cards layer above with subtle stroke.
 */
class PageBackground : public QWidget {
    Q_OBJECT
public:
    explicit PageBackground(QWidget *parent = nullptr) : QWidget(parent) {
        setAutoFillBackground(false);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(rect(), Theme::instance().bgPage());
    }
};

MainWindow::MainWindow(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QMainWindow(parent), m_db(db), m_ai(ai)
{
    setWindowTitle(I18n::t("app.window_title"));
    resize(1320, 840);
    setMinimumSize(1000, 660);

    m_bg = new PageBackground;
    setCentralWidget(m_bg);

    auto *root = new QHBoxLayout(m_bg);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_sidebar = new Sidebar;
    root->addWidget(m_sidebar);

    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("QStackedWidget{background:transparent;}");

    m_calendarPage  = new CalendarPage(m_db, m_ai);
    m_analyticsPage = new AnalyticsPage(m_db);
    m_chatPage      = new ChatPage(m_db, m_ai);

    m_stack->addWidget(m_calendarPage);
    m_stack->addWidget(m_analyticsPage);
    m_stack->addWidget(m_chatPage);
    root->addWidget(m_stack, 1);

    connect(m_sidebar, &Sidebar::navigated, this, [this](int idx){
        m_stack->setCurrentIndex(idx);
        if (idx == 1) m_analyticsPage->refresh();
    });
    connect(m_sidebar, &Sidebar::themeToggleRequested,    this, &MainWindow::onThemeToggle);
    connect(m_sidebar, &Sidebar::settingsRequested,       this, &MainWindow::onSettings);
    connect(m_sidebar, &Sidebar::languageToggleRequested, this, []{ I18n::instance().toggle(); });

    connect(&Theme::instance(), &Theme::changed,        this, &MainWindow::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, [this]{
        setWindowTitle(I18n::t("app.window_title"));
    });
    applyTheme();

    // V4 § 5.4 first-run onboarding (deferred so the main window paints first)
    QTimer::singleShot(120, this, &MainWindow::showOnboardingIfFirstRun);
}

void MainWindow::showOnboardingIfFirstRun() {
    QSettings s;
    if (s.value("onboarded", false).toBool()) return;
    OnboardingDialog dlg(m_ai, this);
    dlg.exec();
    // After onboarding (whether finished or skipped), language may have changed
    // so refresh the window title.
    setWindowTitle(I18n::t("app.window_title"));
}

void MainWindow::onThemeToggle() { Theme::instance().toggle(); }

void MainWindow::onSettings() {
    SettingsDialog dlg(m_ai, m_db->filePath(), this);
    dlg.exec();
}

void MainWindow::applyTheme() {
    setStyleSheet(Theme::instance().globalStylesheet());
    if (m_bg) m_bg->update();
}

} // namespace timemaster

#include "MainWindow.moc"
