#include "CalendarPage.h"
#include "Theme.h"
#include "IconRenderer.h"
#include "MonthView.h"
#include "TimeGridView.h"
#include "EventDialog.h"
#include "AiHistoryDialog.h"
#include "AiResultsDialog.h"
#include "widgets/EmptyState.h"
#include "widgets/ShadowEffect.h"
#include "../core/Database.h"
#include "../core/DeepSeekClient.h"
#include "../core/I18n.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QShortcut>
#include <QKeySequence>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>

namespace timemaster {

CalendarPage::CalendarPage(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QWidget(parent), m_db(db), m_ai(ai)
{
    setAttribute(Qt::WA_StyledBackground, false);
    m_currentDate = QDate::currentDate();
    buildUi();
    buildCmdKDialog();
    applyLanguage();
    applyTheme();

    connect(&Theme::instance(), &Theme::changed,        this, &CalendarPage::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, &CalendarPage::applyLanguage);

    connect(m_db, &Database::eventsChanged, this, &CalendarPage::refresh);
    connect(m_db, &Database::eventsChanged, this, &CalendarPage::refreshEmptyState);
    connect(m_ai, &DeepSeekClient::parseFinished, this, &CalendarPage::onParseFinished);
    connect(m_ai, &DeepSeekClient::parseError,    this, &CalendarPage::onParseError);

    refresh();
    refreshEmptyState();
}

void CalendarPage::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);

    // ============ Single-row header (V4 § 6.2) ============
    auto *headerWidget = new QWidget;
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(2, 0, 2, 0);
    headerLayout->setSpacing(8);

    m_btnPrev = new QPushButton;
    m_btnPrev->setObjectName("NavArrow");
    m_btnPrev->setFixedSize(34, 34);
    m_btnPrev->setIconSize(QSize(16, 16));
    m_btnPrev->setCursor(Qt::PointingHandCursor);
    connect(m_btnPrev, &QPushButton::clicked, this, &CalendarPage::goPrev);
    headerLayout->addWidget(m_btnPrev);

    m_btnNext = new QPushButton;
    m_btnNext->setObjectName("NavArrow");
    m_btnNext->setFixedSize(34, 34);
    m_btnNext->setIconSize(QSize(16, 16));
    m_btnNext->setCursor(Qt::PointingHandCursor);
    connect(m_btnNext, &QPushButton::clicked, this, &CalendarPage::goNext);
    headerLayout->addWidget(m_btnNext);

    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("CalendarTitle");
    QFont tf;
    tf.setPointSize(17);     // ~22px
    tf.setWeight(QFont::DemiBold);  // 600
    m_titleLabel->setFont(tf);
    m_titleLabel->setMinimumWidth(220);
    m_titleLabel->setContentsMargins(8, 0, 8, 0);
    headerLayout->addWidget(m_titleLabel);

    m_btnToday = new QPushButton;
    m_btnToday->setObjectName("TodayBtn");
    m_btnToday->setMinimumSize(68, 30);
    m_btnToday->setCursor(Qt::PointingHandCursor);
    connect(m_btnToday, &QPushButton::clicked, this, &CalendarPage::goToday);
    headerLayout->addWidget(m_btnToday);

    headerLayout->addStretch();

    // View segment Day/Week/Month
    auto *segWrap = new QWidget;
    segWrap->setObjectName("ViewSegment");
    auto *segLayout = new QHBoxLayout(segWrap);
    segLayout->setContentsMargins(3, 3, 3, 3);
    segLayout->setSpacing(2);

    m_btnDay   = new QPushButton;
    m_btnWeek  = new QPushButton;
    m_btnMonth = new QPushButton;
    for (auto *b : {m_btnDay, m_btnWeek, m_btnMonth}) {
        b->setObjectName("ViewSegBtn");
        b->setCheckable(true);
        b->setMinimumSize(54, 26);
        b->setCursor(Qt::PointingHandCursor);
        segLayout->addWidget(b);
    }
    headerLayout->addWidget(segWrap);

    connect(m_btnDay,   &QPushButton::clicked, this, [this]{ setView(CalendarView::Day); });
    connect(m_btnWeek,  &QPushButton::clicked, this, [this]{ setView(CalendarView::Week); });
    connect(m_btnMonth, &QPushButton::clicked, this, [this]{ setView(CalendarView::Month); });

    m_btnAdd = new QPushButton;
    m_btnAdd->setProperty("class", "primary");
    m_btnAdd->setMinimumSize(120, 32);
    m_btnAdd->setCursor(Qt::PointingHandCursor);
    connect(m_btnAdd, &QPushButton::clicked, this, [this]{ openCreateDialog(); });
    headerLayout->addWidget(m_btnAdd);

    root->addWidget(headerWidget);

    // ============ View area + EmptyState overlay ============
    m_viewCard = new QWidget;
    m_viewCard->setObjectName("ViewCard");

    // Use a stacked layout so EmptyState sits over the views when needed
    auto *overlay = new QStackedLayout(m_viewCard);
    overlay->setContentsMargins(0, 0, 0, 0);
    overlay->setStackingMode(QStackedLayout::StackAll);

    // Layer 1: stack of views
    auto *viewsHost = new QWidget;
    auto *viewsLay = new QVBoxLayout(viewsHost);
    viewsLay->setContentsMargins(0, 0, 0, 0);

    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("QStackedWidget{background:transparent;}");
    m_monthView = new MonthView(this);
    m_weekView  = new TimeGridView(TimeGridView::WeekMode, this);
    m_dayView   = new TimeGridView(TimeGridView::DayMode, this);
    m_stack->addWidget(m_monthView);
    m_stack->addWidget(m_weekView);
    m_stack->addWidget(m_dayView);
    viewsLay->addWidget(m_stack);

    overlay->addWidget(viewsHost);

    // Layer 2: EmptyState (initially hidden)
    m_emptyState = new EmptyState;
    overlay->addWidget(m_emptyState);

    root->addWidget(m_viewCard, 1);

    connect(m_monthView, &MonthView::dateClicked,     this, &CalendarPage::onMonthDateClicked);
    connect(m_monthView, &MonthView::eventClicked,    this, &CalendarPage::onEventClicked);
    connect(m_monthView, &MonthView::overflowClicked, this, &CalendarPage::onMonthOverflowClicked);

    connect(m_weekView, &TimeGridView::timeSlotClicked, this, &CalendarPage::onTimeSlotClicked);
    connect(m_weekView, &TimeGridView::eventClicked,    this, &CalendarPage::onEventClicked);
    connect(m_dayView,  &TimeGridView::timeSlotClicked, this, &CalendarPage::onTimeSlotClicked);
    connect(m_dayView,  &TimeGridView::eventClicked,    this, &CalendarPage::onEventClicked);

    // Shortcuts
    auto bind = [this](const char *key, std::function<void()> fn) {
        auto *sc = new QShortcut(QKeySequence(key), this);
        connect(sc, &QShortcut::activated, this, fn);
    };
    bind("Ctrl+N", [this]{ openCreateDialog(); });
    bind("Ctrl+K", [this]{ openCmdKPalette(); });
    bind("Ctrl+Z", [this]{ onHistoryClicked(); });
    bind("T",      [this]{ goToday(); });
    bind("Left",   [this]{ goPrev(); });
    bind("Right",  [this]{ goNext(); });
    bind("1",      [this]{ setView(CalendarView::Day); });
    bind("2",      [this]{ setView(CalendarView::Week); });
    bind("3",      [this]{ setView(CalendarView::Month); });

    setView(CalendarView::Month);
}

void CalendarPage::buildCmdKDialog() {
    m_cmdKDialog = new QDialog(this);
    m_cmdKDialog->setObjectName("CmdKDialog");
    m_cmdKDialog->setWindowFlag(Qt::FramelessWindowHint);
    m_cmdKDialog->setAttribute(Qt::WA_TranslucentBackground, false);
    m_cmdKDialog->setMinimumWidth(600);

    auto *root = new QVBoxLayout(m_cmdKDialog);
    root->setContentsMargins(20, 18, 20, 16);
    root->setSpacing(10);

    m_cmdKTitle = new QLabel;
    m_cmdKTitle->setObjectName("CmdKTitle");
    QFont tf; tf.setPointSize(11); tf.setWeight(QFont::DemiBold);
    m_cmdKTitle->setFont(tf);
    root->addWidget(m_cmdKTitle);

    auto *row = new QHBoxLayout;
    row->setSpacing(8);

    m_parseInput = new QLineEdit;
    m_parseInput->setObjectName("CmdKInput");
    m_parseInput->setMinimumHeight(42);
    row->addWidget(m_parseInput, 1);

    m_parseButton = new QPushButton;
    m_parseButton->setProperty("class", "primary");
    m_parseButton->setMinimumSize(110, 42);
    m_parseButton->setCursor(Qt::PointingHandCursor);
    row->addWidget(m_parseButton);

    m_historyButton = new QPushButton;
    m_historyButton->setObjectName("CmdKHistoryBtn");
    m_historyButton->setMinimumSize(96, 42);
    m_historyButton->setCursor(Qt::PointingHandCursor);
    m_historyButton->setIconSize(QSize(14, 14));
    row->addWidget(m_historyButton);

    root->addLayout(row);

    auto *footer = new QHBoxLayout;
    m_cmdKHint = new QLabel;
    m_cmdKHint->setObjectName("CmdKHint");
    footer->addWidget(m_cmdKHint);
    footer->addStretch();
    m_parseStatus = new QLabel;
    m_parseStatus->setObjectName("CmdKStatus");
    footer->addWidget(m_parseStatus);
    root->addLayout(footer);

    connect(m_parseButton,  &QPushButton::clicked,    this, &CalendarPage::onParseClicked);
    connect(m_parseInput,   &QLineEdit::returnPressed, this, &CalendarPage::onParseClicked);
    connect(m_historyButton,&QPushButton::clicked,    this, &CalendarPage::onHistoryClicked);
}

void CalendarPage::openCmdKPalette() {
    if (!m_cmdKDialog) return;
    m_parseStatus->clear();
    m_parseInput->clear();
    m_parseInput->setFocus();
    // Center over main window
    if (auto *top = window()) {
        QRect g = top->geometry();
        QPoint c(g.center().x() - 300, g.top() + 120);
        m_cmdKDialog->move(c);
    }
    m_cmdKDialog->exec();
}

void CalendarPage::applyLanguage() {
    if (m_btnToday)  m_btnToday->setText(I18n::t("calendar.today"));
    if (m_btnDay)    m_btnDay->setText(I18n::t("calendar.view.day"));
    if (m_btnWeek)   m_btnWeek->setText(I18n::t("calendar.view.week"));
    if (m_btnMonth)  m_btnMonth->setText(I18n::t("calendar.view.month"));
    if (m_btnAdd)    m_btnAdd->setText(I18n::t("calendar.new_event"));

    if (m_cmdKDialog)   m_cmdKDialog->setWindowTitle(I18n::t("calendar.cmdk.title"));
    if (m_cmdKTitle)    m_cmdKTitle->setText(I18n::t("calendar.cmdk.title"));
    if (m_cmdKHint)     m_cmdKHint->setText(I18n::t("calendar.cmdk.hint"));
    if (m_parseInput)   m_parseInput->setPlaceholderText(I18n::t("calendar.cmdk.placeholder"));
    if (m_parseButton)  m_parseButton->setText(I18n::t("calendar.cmdk.parse"));
    if (m_historyButton)m_historyButton->setText(QStringLiteral("  ") + I18n::t("calendar.cmdk.history"));

    // Refresh header title (month names depend on language)
    updateHeader();
    refreshEmptyState();
}

void CalendarPage::applyTheme() {
    auto &t = Theme::instance();

    QString brand        = t.brand().name();
    QString textPrim     = t.textPrimary().name();
    QString textSec      = t.textSecondary().name();
    QString strokeR      = t.strokeRgba();
    QString cardBg       = t.cardBgRgba();
    QString componentBg  = t.componentBgRgba();
    QString hoverBg      = t.cardBgHoverRgba();

    setStyleSheet(t.globalStylesheet() + QString(R"(
        QLabel#CalendarTitle {
            color: %3;
            background: transparent;
            font-weight: 600;
            letter-spacing: -0.2px;
        }
        QPushButton#NavArrow {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 0;
        }
        QPushButton#NavArrow:hover {
            background-color: %6;
        }
        QPushButton#TodayBtn {
            background-color: transparent;
            border: 1px solid %2;
            border-radius: 8px;
            color: %5;
            font-weight: 500;
            padding: 0 14px;
        }
        QPushButton#TodayBtn:hover {
            background-color: %6;
            color: %3;
        }
        QWidget#ViewSegment {
            background-color: %4;
            border: 1px solid %2;
            border-radius: 8px;
        }
        QPushButton#ViewSegBtn {
            background-color: transparent;
            border: none;
            border-radius: 6px;
            color: %5;
            font-weight: 500;
            padding: 0 8px;
        }
        QPushButton#ViewSegBtn:hover {
            background-color: rgba(120,120,140,0.10);
            color: %3;
        }
        QPushButton#ViewSegBtn:checked {
            background-color: %7;
            color: white;
            font-weight: 600;
        }
        QWidget#ViewCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }

        /* Cmd+K palette */
        QDialog#CmdKDialog {
            background-color: %1;
            border: 1px solid %2;
        }
        QLabel#CmdKTitle {
            color: %5;
            background: transparent;
            font-size: 12px;
            font-weight: 600;
            letter-spacing: 0.4px;
        }
        QLabel#CmdKHint {
            color: %8;
            background: transparent;
            font-size: 12px;
        }
        QLabel#CmdKStatus {
            color: %5;
            background: transparent;
            font-size: 12px;
        }
        QLineEdit#CmdKInput {
            background-color: %4;
            color: %3;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 0 12px;
            font-size: 14px;
        }
        QLineEdit#CmdKInput:focus { border: 1px solid %7; }
        QPushButton#CmdKHistoryBtn {
            background-color: %4;
            border: 1px solid %2;
            border-radius: 8px;
            color: %5;
            font-weight: 500;
        }
        QPushButton#CmdKHistoryBtn:hover { background-color: %6; color: %3; }
    )")
    /*1*/.arg(cardBg)
    /*2*/.arg(strokeR)
    /*3*/.arg(textPrim)
    /*4*/.arg(componentBg)
    /*5*/.arg(textSec)
    /*6*/.arg(hoverBg)
    /*7*/.arg(brand)
    /*8*/.arg(t.textPlaceholder().name()));

    m_btnPrev->setIcon(IconRenderer::icon(IconRenderer::ArrowLeft,  t.textPrimary(), 16));
    m_btnNext->setIcon(IconRenderer::icon(IconRenderer::ArrowRight, t.textPrimary(), 16));
    if (m_historyButton)
        m_historyButton->setIcon(IconRenderer::icon(IconRenderer::History, t.textSecondary(), 14));
    if (m_cmdKDialog) m_cmdKDialog->setStyleSheet(styleSheet());
    // V4 § 7.2 — subtle shadow on the main view card
    if (m_viewCard) {
        ShadowEffect::apply(m_viewCard, ShadowEffect::Subtle, t.mode() == Theme::Dark);
    }
}

void CalendarPage::refreshEmptyState() {
    if (!m_db || !m_emptyState) return;
    bool isEmpty = m_db->getAllEvents().isEmpty();
    if (isEmpty) {
        m_emptyState->setTitle(I18n::t("empty.cal.title"));
        m_emptyState->setSubtitle(I18n::t("empty.cal.subtitle"));
        m_emptyState->setProgress(QString());
        m_emptyState->clearActions();
        m_emptyState->addAction(I18n::t("empty.cal.tmpl.morning"),  [this]{ insertTemplate(0); });
        m_emptyState->addAction(I18n::t("empty.cal.tmpl.deepwork"), [this]{ insertTemplate(1); });
        m_emptyState->addAction(I18n::t("empty.cal.tmpl.review"),   [this]{ insertTemplate(2); });
        m_emptyState->show();
        m_emptyState->raise();
    } else {
        m_emptyState->hide();
    }
}

void CalendarPage::insertTemplate(int which) {
    // Builds a small starter set of events so the calendar isn't blank.
    QDate today = QDate::currentDate();
    QString batchId = m_db ? m_db->createAiBatch(QStringLiteral("template"), QStringLiteral("template")) : QString();

    auto addEvent = [&](const QString &titleEn, const QString &titleZh,
                        const QDate &d, int sH, int sM, int durMin,
                        EventCategory cat, EventColor color)
    {
        CalendarEvent e;
        e.id          = Database::generateId();
        e.title       = I18n::instance().isEnglish() ? titleEn : titleZh;
        e.startDate   = QDateTime(d, QTime(sH, sM));
        e.endDate     = e.startDate.addSecs(durMin * 60);
        e.allDay      = false;
        e.color       = color;
        e.category    = cat;
        e.priority    = EventPriority::Normal;
        e.source      = EventSource::Manual;
        e.aiBatchId   = batchId;
        e.createdAt   = QDateTime::currentDateTime();
        e.updatedAt   = e.createdAt;
        if (m_db) m_db->insertEvent(e);
    };

    switch (which) {
    case 0: // Morning routine — three days
        for (int i = 0; i < 3; ++i) {
            QDate d = today.addDays(i);
            addEvent("Wake up & stretch", "起床 & 拉伸", d, 7, 0, 20,
                     EventCategory::Exercise, EventColor::Green);
            addEvent("Plan the day",      "规划今天",   d, 8, 0, 15,
                     EventCategory::Personal, EventColor::Indigo);
        }
        break;
    case 1: // Deep work — today + tomorrow morning
        addEvent("Deep work block",       "深度工作",   today, 9, 30, 90,
                 EventCategory::Work, EventColor::Blue);
        addEvent("Deep work block",       "深度工作",   today.addDays(1), 9, 30, 90,
                 EventCategory::Work, EventColor::Blue);
        break;
    case 2: // Weekly review — this Friday
    default: {
        int dow = today.dayOfWeek();           // Mon=1 .. Sun=7
        int daysToFri = (5 - dow + 7) % 7;      // 0..6
        QDate fri = today.addDays(daysToFri);
        addEvent("Weekly review",         "周复盘",     fri, 17, 0, 45,
                 EventCategory::Personal, EventColor::Purple);
        break;
    }
    }
    refresh();
    refreshEmptyState();
}

void CalendarPage::setView(CalendarView v) {
    m_view = v;
    switchViewWidget();
    m_btnDay->setChecked(v == CalendarView::Day);
    m_btnWeek->setChecked(v == CalendarView::Week);
    m_btnMonth->setChecked(v == CalendarView::Month);
    refresh();
    if (v != CalendarView::Month) {
        auto *grid = (v == CalendarView::Week) ? m_weekView : m_dayView;
        grid->scrollToHour(7);
    }
}

void CalendarPage::switchViewWidget() {
    switch (m_view) {
        case CalendarView::Month: m_stack->setCurrentWidget(m_monthView); break;
        case CalendarView::Week:  m_stack->setCurrentWidget(m_weekView);  break;
        case CalendarView::Day:   m_stack->setCurrentWidget(m_dayView);   break;
    }
}

void CalendarPage::goToday() {
    m_currentDate = QDate::currentDate();
    refresh();
}

void CalendarPage::goNext() {
    switch (m_view) {
        case CalendarView::Day:   m_currentDate = m_currentDate.addDays(1);   break;
        case CalendarView::Week:  m_currentDate = m_currentDate.addDays(7);   break;
        case CalendarView::Month: m_currentDate = m_currentDate.addMonths(1); break;
    }
    refresh();
}

void CalendarPage::goPrev() {
    switch (m_view) {
        case CalendarView::Day:   m_currentDate = m_currentDate.addDays(-1);   break;
        case CalendarView::Week:  m_currentDate = m_currentDate.addDays(-7);   break;
        case CalendarView::Month: m_currentDate = m_currentDate.addMonths(-1); break;
    }
    refresh();
}

void CalendarPage::refresh() {
    QDate gridStart, gridEnd;
    switch (m_view) {
        case CalendarView::Month: {
            QDate first(m_currentDate.year(), m_currentDate.month(), 1);
            int dow = first.dayOfWeek() - 1;
            gridStart = first.addDays(-dow);
            gridEnd   = gridStart.addDays(42);
            break;
        }
        case CalendarView::Week: {
            int dow = m_currentDate.dayOfWeek() - 1;
            gridStart = m_currentDate.addDays(-dow);
            gridEnd   = gridStart.addDays(7);
            break;
        }
        case CalendarView::Day: {
            gridStart = m_currentDate;
            gridEnd   = m_currentDate.addDays(1);
            break;
        }
    }

    QDateTime startDt(gridStart, QTime(0, 0, 0));
    QDateTime endDt(gridEnd, QTime(23, 59, 59));
    m_events = m_db->getEventsByRange(startDt, endDt);

    m_monthView->setCurrentDate(m_currentDate);
    m_monthView->setEvents(m_events);
    m_weekView->setCurrentDate(m_currentDate);
    m_weekView->setEvents(m_events);
    m_dayView->setCurrentDate(m_currentDate);
    m_dayView->setEvents(m_events);

    updateHeader();
}

QString CalendarPage::monthName(int month) const {
    static const char *keys[] = {
        "month.jan","month.feb","month.mar","month.apr","month.may","month.jun",
        "month.jul","month.aug","month.sep","month.oct","month.nov","month.dec"
    };
    if (month < 1 || month > 12) return QString::number(month);
    return I18n::t(keys[month - 1]);
}

void CalendarPage::updateHeader() {
    if (!m_titleLabel) return;
    QString title;
    int y  = m_currentDate.year();
    int mo = m_currentDate.month();
    int d  = m_currentDate.day();
    bool en = I18n::instance().isEnglish();

    switch (m_view) {
        case CalendarView::Month:
            title = en
                ? QString("%1 %2").arg(monthName(mo)).arg(y)
                : QString("%1 年 %2").arg(y).arg(monthName(mo));
            break;
        case CalendarView::Week: {
            int dow = m_currentDate.dayOfWeek() - 1;
            QDate ws = m_currentDate.addDays(-dow);
            QDate we = ws.addDays(6);
            if (en) {
                if (ws.month() == we.month()) {
                    title = QString("%1 %2 — %3, %4").arg(monthName(ws.month()))
                              .arg(ws.day()).arg(we.day()).arg(ws.year());
                } else {
                    title = QString("%1 %2 — %3 %4").arg(monthName(ws.month()))
                              .arg(ws.day()).arg(monthName(we.month())).arg(we.day());
                }
            } else {
                if (ws.month() == we.month()) {
                    title = QString("%1 年 %2 %3 — %4 日").arg(ws.year())
                              .arg(monthName(ws.month())).arg(ws.day()).arg(we.day());
                } else {
                    title = QString("%1 %2日 — %3 %4日")
                              .arg(monthName(ws.month())).arg(ws.day())
                              .arg(monthName(we.month())).arg(we.day());
                }
            }
            break;
        }
        case CalendarView::Day:
            title = en
                ? QString("%1 %2, %3").arg(monthName(mo)).arg(d).arg(y)
                : QString("%1 年 %2 %3 日").arg(y).arg(monthName(mo)).arg(d);
            break;
    }
    m_titleLabel->setText(title);
}

void CalendarPage::openCreateDialog(const QDateTime &dt) {
    QDateTime start = dt;
    if (!start.isValid()) {
        QDateTime n = QDateTime::currentDateTime();
        start = QDateTime(n.date(), QTime(qBound(0, n.time().hour() + 1, 22), 0));
    }
    EventDialog dlg(this);
    dlg.setupForCreate(start);
    if (dlg.exec() == QDialog::Accepted) {
        auto e = dlg.result();
        if (!m_db->insertEvent(e)) {
            QMessageBox::warning(this, I18n::t("common.error"), I18n::t("event.save_failed"));
        }
    }
}

void CalendarPage::onEventClicked(const CalendarEvent &event) {
    EventDialog dlg(this);
    dlg.setupForEdit(event);
    connect(&dlg, &EventDialog::requestDelete, this, [this](const QString &id) {
        m_db->deleteEvent(id);
    });
    if (dlg.exec() == QDialog::Accepted) {
        auto e = dlg.result();
        if (!m_db->updateEvent(e)) {
            QMessageBox::warning(this, I18n::t("common.error"), I18n::t("event.update_failed"));
        }
    }
}

void CalendarPage::onTimeSlotClicked(const QDateTime &dt) {
    openCreateDialog(dt);
}

void CalendarPage::onMonthDateClicked(const QDate &d) {
    QDateTime now = QDateTime::currentDateTime();
    QTime t = (d == QDate::currentDate())
        ? QTime(qBound(0, now.time().hour() + 1, 22), 0)
        : QTime(9, 0);
    openCreateDialog(QDateTime(d, t));
}

void CalendarPage::onMonthOverflowClicked(const QDate &d, const QList<CalendarEvent> &events) {
    QDialog dlg(this);
    dlg.setWindowTitle(QString("%1 — %2").arg(d.toString("yyyy-MM-dd")).arg(events.size()));
    dlg.setMinimumSize(400, 480);
    dlg.setStyleSheet(Theme::instance().globalStylesheet());
    auto *layout = new QVBoxLayout(&dlg);

    auto *list = new QListWidget(&dlg);
    auto pal = Theme::instance().palette();
    QString allDayLabel = I18n::instance().isEnglish() ? QStringLiteral("All day")
                                                       : QStringLiteral("全天");
    for (const auto &e : events) {
        auto *item = new QListWidgetItem();
        QString time = e.allDay ? allDayLabel : e.startDate.toString("HH:mm");
        item->setText(QString("  %1   %2").arg(time, e.title));
        QColor c = pal[e.color].text;
        item->setForeground(c);
        item->setData(Qt::UserRole, e.id);
        item->setSizeHint(QSize(0, 38));
        list->addItem(item);
    }
    layout->addWidget(list);

    connect(list, &QListWidget::itemDoubleClicked, this, [this, &dlg, events](QListWidgetItem *item) {
        QString id = item->data(Qt::UserRole).toString();
        for (const auto &e : events) {
            if (e.id == id) {
                dlg.accept();
                onEventClicked(e);
                return;
            }
        }
    });

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(bb);

    dlg.exec();
}

void CalendarPage::onParseClicked() {
    QString text = m_parseInput->text().trimmed();
    if (text.isEmpty()) return;
    if (!m_ai->hasApiKey()) {
        QMessageBox::information(this, I18n::t("common.info"), I18n::t("chat.api.missing"));
        return;
    }
    m_pendingParseText = text;
    m_parseStatus->setText(I18n::t("calendar.parse.parsing"));
    m_parseButton->setEnabled(false);
    m_ai->parseSchedule(text);
}

void CalendarPage::onParseFinished(const QList<ScheduleSuggestion> &items) {
    m_parseButton->setEnabled(true);
    if (items.isEmpty()) {
        m_parseStatus->setText(I18n::t("calendar.parse.none"));
        return;
    }

    AiResultsDialog dlg(items, this);
    if (dlg.exec() != QDialog::Accepted) {
        m_parseStatus->setText(I18n::t("calendar.parse.cancelled"));
        return;
    }
    auto picked = dlg.selectedSuggestions();
    if (picked.isEmpty()) {
        m_parseStatus->setText(I18n::t("calendar.parse.no_selection"));
        return;
    }

    QString batchId = m_db->createAiBatch(m_pendingParseText, "parse");
    int added = 0;
    for (const auto &s : picked) {
        CalendarEvent e;
        e.id          = Database::generateId();
        e.title       = s.title;
        e.description = s.description;
        e.startDate   = s.startDate.isValid() ? s.startDate : QDateTime::currentDateTime();
        e.endDate     = s.endDate.isValid()   ? s.endDate
                          : e.startDate.addSecs(60 * qMax(15, s.durationMinutes));
        e.allDay      = s.allDay;
        e.color       = s.color;
        e.category    = s.category;
        e.priority    = s.priority;
        e.source      = EventSource::AiParse;
        e.aiBatchId   = batchId;
        e.createdAt   = QDateTime::currentDateTime();
        e.updatedAt   = e.createdAt;
        if (m_db->insertEvent(e)) added++;
    }
    m_parseStatus->setText(I18n::t("calendar.parse.imported_fmt").arg(added));
    m_parseInput->clear();
    m_pendingParseText.clear();
    if (m_cmdKDialog && m_cmdKDialog->isVisible()) {
        m_cmdKDialog->accept();
    }
}

void CalendarPage::onParseError(const QString &msg) {
    m_parseButton->setEnabled(true);
    m_parseStatus->setText(I18n::t("calendar.parse.failed"));
    QMessageBox::warning(this, I18n::t("calendar.parse.failed"), msg);
}

void CalendarPage::onHistoryClicked() {
    AiHistoryDialog dlg(m_db, this);
    dlg.exec();
}

} // namespace timemaster
