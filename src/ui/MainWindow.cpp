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
#include <QVBoxLayout>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QLinearGradient>
#include <QRadialGradient>

namespace timemaster {

class PageBackground : public QWidget {
    Q_OBJECT
public:
    explicit PageBackground(QWidget *parent = nullptr) : QWidget(parent) {
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        auto &t = Theme::instance();
        QLinearGradient grad(QPointF(0, 0), QPointF(width(), height()));
        grad.setColorAt(0.0, t.bgPageTop());
        grad.setColorAt(1.0, t.bgPageBottom());
        p.fillRect(rect(), grad);

        QColor glow1 = t.brand();
        glow1.setAlphaF(0.06);
        QRadialGradient r1(QPointF(width() * 0.88, height() * 0.06),
                           qMax(width(), height()) * 0.5);
        r1.setColorAt(0.0, glow1);
        r1.setColorAt(1.0, Qt::transparent);
        p.fillRect(rect(), r1);

        QColor glow2 = t.accent();
        glow2.setAlphaF(0.05);
        QRadialGradient r2(QPointF(width() * 0.08, height() * 0.94),
                           qMax(width(), height()) * 0.55);
        r2.setColorAt(0.0, glow2);
        r2.setColorAt(1.0, Qt::transparent);
        p.fillRect(rect(), r2);
    }
};

MainWindow::MainWindow(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QMainWindow(parent), m_db(db), m_ai(ai)
{
    setWindowTitle("时间管理大师 · Time Master");
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

    m_calendarPage = new CalendarPage(m_db, m_ai);
    m_analyticsPage = new AnalyticsPage(m_db);
    m_chatPage = new ChatPage(m_db, m_ai);

    m_stack->addWidget(m_calendarPage);
    m_stack->addWidget(m_analyticsPage);
    m_stack->addWidget(m_chatPage);
    root->addWidget(m_stack, 1);

    connect(m_sidebar, &Sidebar::navigated, this, [this](int idx){
        m_stack->setCurrentIndex(idx);
        if (idx == 1) m_analyticsPage->refresh();
    });
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
    if (m_bg) m_bg->update();
}

} // namespace timemaster

#include "MainWindow.moc"
