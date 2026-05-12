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
#include "../core/Preferences.h"

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
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QTimer>

namespace timemaster {

CalendarPage::CalendarPage(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QWidget(parent), m_db(db), m_ai(ai)
{
    setAttribute(Qt::WA_StyledBackground, false);
    m_currentDate = QDate::currentDate();
    buildUi();
    applyLanguage();
    applyTheme();

    connect(&Theme::instance(), &Theme::changed,        this, &CalendarPage::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, &CalendarPage::applyLanguage);
    // V4.3 #8 — 周起始日变化后，所有 view 的可见范围都得重算
    connect(&Preferences::instance(), &Preferences::weekStartChanged, this, &CalendarPage::refresh);

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

    // ============ Row 1: title + nav + view segment + new event ============
    // V4.2 layout: [Title]  [◄] [►] [Today]  ——stretch——  [Day|Week|Month]  [+ New Event]
    auto *headerWidget = new QWidget;
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(2, 0, 2, 0);
    headerLayout->setSpacing(8);

    // 日期范围标题最左
    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("CalendarTitle");
    QFont tf;
    tf.setPointSize(19);     // V4.2 #4: bumped from 17 -> 19 (~25px)
    tf.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(tf);
    m_titleLabel->setMinimumWidth(240);
    m_titleLabel->setContentsMargins(0, 0, 12, 0);
    headerLayout->addWidget(m_titleLabel);

    // ◄ ► 紧跟标题
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

    // 今天按钮紧贴 ◄►
    m_btnToday = new QPushButton;
    m_btnToday->setObjectName("TodayBtn");
    m_btnToday->setMinimumSize(68, 30);
    m_btnToday->setCursor(Qt::PointingHandCursor);
    connect(m_btnToday, &QPushButton::clicked, this, &CalendarPage::goToday);
    headerLayout->addWidget(m_btnToday);

    // V4.2: AI 输入栏合并到同一行 header 里，占据中间扩展空间。
    // m_aiBar 在下面构造完成后用 insertWidget 插到 segWrap 之前。

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

    // ============ Row 2 (V4.2 #5): narrow decorative pill, NOT full-width =====
    // The pill is the only thing visible by default. Clicking it (or Ctrl+K)
    // reveals a floating Spotlight-style card that sits *above* the calendar
    // grid — handled below as m_aiExpanded.
    m_aiBar = new QWidget;
    m_aiBar->setObjectName("AiBarHost");
    auto *aiBarLayout = new QHBoxLayout(m_aiBar);
    aiBarLayout->setContentsMargins(0, 0, 0, 0);
    aiBarLayout->setSpacing(10);

    aiBarLayout->addStretch(1);

    // Inner pill: fixed max width so it reads as decorative, not as an input bar
    auto *pillWrap = new QWidget;
    pillWrap->setObjectName("AiPillWrap");
    pillWrap->setMaximumWidth(520);
    pillWrap->setMinimumWidth(360);
    auto *pillRow = new QHBoxLayout(pillWrap);
    pillRow->setContentsMargins(0, 0, 0, 0);
    pillRow->setSpacing(8);

    m_aiCompactInput = new QLineEdit;
    m_aiCompactInput->setObjectName("AiCompactInput");
    m_aiCompactInput->setMinimumHeight(38);
    m_aiCompactInput->setReadOnly(true);
    m_aiCompactInput->installEventFilter(this);
    m_aiCompactInput->setCursor(Qt::PointingHandCursor);
    m_aiCompactInput->setFocusPolicy(Qt::ClickFocus);
    pillRow->addWidget(m_aiCompactInput, 1);

    m_historyButton = new QPushButton;
    m_historyButton->setObjectName("HistoryBtn");
    m_historyButton->setCursor(Qt::PointingHandCursor);
    m_historyButton->setMinimumSize(38, 38);
    m_historyButton->setMaximumWidth(44);
    m_historyButton->setIconSize(QSize(16, 16));
    m_historyButton->setFocusPolicy(Qt::NoFocus);
    m_historyButton->setToolTip(I18n::t("calendar.ai.history"));
    connect(m_historyButton, &QPushButton::clicked, this, &CalendarPage::onHistoryClicked);
    pillRow->addWidget(m_historyButton);

    aiBarLayout->addWidget(pillWrap);
    aiBarLayout->addStretch(1);

    // V4.2: 合并到 header 同一行，插入到 [Today] 与 [Day|Week|Month] 之间，
    // 用 stretch factor 1 占据中间所有空间。pill 内部用 addStretch 保持居中。
    headerLayout->insertWidget(headerLayout->indexOf(segWrap), m_aiBar, 1);

    // ============ Floating AI panel (V4.2 #5): popup overlay, NOT in layout ===
    // Parented to `this` but not added to any layout — we position it manually
    // below the compact pill via positionAiPopup() and raise() it on top.
    m_aiExpanded = new QWidget(this);
    m_aiExpanded->setObjectName("AiExpandedPanel");
    m_aiExpanded->hide();
    m_aiExpanded->setAttribute(Qt::WA_StyledBackground, true);
    auto *expLay = new QVBoxLayout(m_aiExpanded);
    expLay->setContentsMargins(18, 14, 14, 14);
    expLay->setSpacing(10);

    auto *expTopRow = new QHBoxLayout;
    expTopRow->setSpacing(8);
    m_aiPanelTitle = new QLabel;
    m_aiPanelTitle->setObjectName("AiPanelTitle");
    expTopRow->addWidget(m_aiPanelTitle);
    expTopRow->addStretch();
    m_aiCloseBtn = new QPushButton("✕");
    m_aiCloseBtn->setObjectName("AiCloseBtn");
    m_aiCloseBtn->setFixedSize(28, 28);
    m_aiCloseBtn->setCursor(Qt::PointingHandCursor);
    m_aiCloseBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_aiCloseBtn, &QPushButton::clicked, this, &CalendarPage::collapseAiPanel);
    expTopRow->addWidget(m_aiCloseBtn);
    expLay->addLayout(expTopRow);

    auto *expMidRow = new QHBoxLayout;
    expMidRow->setSpacing(8);
    m_parseInput = new QLineEdit;
    m_parseInput->setObjectName("AiFullInput");
    m_parseInput->setMinimumHeight(42);
    m_parseInput->installEventFilter(this);
    expMidRow->addWidget(m_parseInput, 1);

    m_parseButton = new QPushButton;
    m_parseButton->setProperty("class", "primary");
    m_parseButton->setMinimumSize(120, 42);
    m_parseButton->setCursor(Qt::PointingHandCursor);
    m_parseButton->setFocusPolicy(Qt::NoFocus);
    connect(m_parseButton, &QPushButton::clicked, this, &CalendarPage::onParseClicked);
    connect(m_parseInput,  &QLineEdit::returnPressed, this, &CalendarPage::onParseClicked);
    expMidRow->addWidget(m_parseButton);
    expLay->addLayout(expMidRow);

    auto *expBotRow = new QHBoxLayout;
    expBotRow->setSpacing(8);
    m_aiHintLabel = new QLabel;
    m_aiHintLabel->setObjectName("AiHintLabel");
    expBotRow->addWidget(m_aiHintLabel);
    expBotRow->addStretch();
    m_parseStatus = new QLabel;
    m_parseStatus->setObjectName("ParseStatusLabel");
    expBotRow->addWidget(m_parseStatus);
    expLay->addLayout(expBotRow);

    // V4.2 #5: drop shadow on the floating card so it reads as elevated
    ShadowEffect::apply(m_aiExpanded, ShadowEffect::Card, Theme::instance().mode() == Theme::Dark);

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
    bind("Ctrl+K", [this]{ expandAiPanel(); });
    bind("Ctrl+Z", [this]{ onHistoryClicked(); });
    bind("Escape", [this]{ if (m_aiExpanded && m_aiExpanded->isVisible()) collapseAiPanel(); });
    bind("T",      [this]{ goToday(); });
    bind("Left",   [this]{ goPrev(); });
    bind("Right",  [this]{ goNext(); });
    bind("1",      [this]{ setView(CalendarView::Day); });
    bind("2",      [this]{ setView(CalendarView::Week); });
    bind("3",      [this]{ setView(CalendarView::Month); });

    setView(CalendarView::Month);
}

void CalendarPage::expandAiPanel() {
    if (!m_aiExpanded) return;
    m_parseStatus->clear();
    m_parseInput->clear();
    positionAiPopup();
    m_aiExpanded->show();
    m_aiExpanded->raise();
    m_parseInput->setFocus();
}

void CalendarPage::collapseAiPanel() {
    if (!m_aiExpanded) return;
    m_aiExpanded->hide();
    m_parseStatus->clear();
}

void CalendarPage::positionAiPopup() {
    if (!m_aiExpanded || !m_aiBar) return;
    // Anchor the popup just below the compact pill, centered horizontally on
    // the page. V4.2 #5 — Spotlight-style drop.
    const int margin = 24;
    const int popupW = qMin(680, width() - margin * 2);
    const int popupH = m_aiExpanded->sizeHint().height();
    int x = (width() - popupW) / 2;
    QPoint barBottomLeft = m_aiBar->mapTo(this, QPoint(0, m_aiBar->height()));
    int y = barBottomLeft.y() + 8;
    m_aiExpanded->setGeometry(x, y, popupW, popupH);
}

void CalendarPage::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    if (m_aiExpanded && m_aiExpanded->isVisible()) {
        positionAiPopup();
    }
}

void CalendarPage::mousePressEvent(QMouseEvent *e) {
    // V4.2 #5: click outside the floating popup closes it (Spotlight behavior).
    if (m_aiExpanded && m_aiExpanded->isVisible()) {
        QRect g = m_aiExpanded->geometry();
        if (!g.contains(e->pos())) {
            collapseAiPanel();
        }
    }
    QWidget::mousePressEvent(e);
}

bool CalendarPage::eventFilter(QObject *o, QEvent *e) {
    // Click on the compact input -> expand the AI panel
    if (o == m_aiCompactInput && e->type() == QEvent::MouseButtonPress) {
        expandAiPanel();
        return true;
    }
    // Auto-collapse when the expanded panel loses focus to something outside it
    if (o == m_parseInput && e->type() == QEvent::FocusOut) {
        // Defer: focus is still moving; only collapse if focus didn't go to a child of m_aiExpanded
        QTimer::singleShot(0, this, [this]{
            if (!m_aiExpanded || !m_aiExpanded->isVisible()) return;
            QWidget *fw = QApplication::focusWidget();
            if (!fw) {
                collapseAiPanel();
                return;
            }
            // If focus moved to anything inside the panel, keep it open
            for (QWidget *w = fw; w; w = w->parentWidget()) {
                if (w == m_aiExpanded) return;
            }
            collapseAiPanel();
        });
    }
    return QWidget::eventFilter(o, e);
}

void CalendarPage::applyLanguage() {
    if (m_btnToday)  m_btnToday->setText(I18n::t("calendar.today"));
    if (m_btnDay)    m_btnDay->setText(I18n::t("calendar.view.day"));
    if (m_btnWeek)   m_btnWeek->setText(I18n::t("calendar.view.week"));
    if (m_btnMonth)  m_btnMonth->setText(I18n::t("calendar.view.month"));
    if (m_btnAdd)    m_btnAdd->setText(I18n::t("calendar.new_event"));

    if (m_aiCompactInput)
        m_aiCompactInput->setPlaceholderText(QStringLiteral("✦  ") + I18n::t("calendar.ai.compact_placeholder"));
    if (m_parseInput)
        m_parseInput->setPlaceholderText(I18n::t("calendar.ai.full_placeholder"));
    if (m_parseButton)
        m_parseButton->setText(I18n::t("calendar.ai.parse"));
    if (m_historyButton)
        m_historyButton->setText(QString());  // V4.2 #5: icon-only in pill mode
    if (m_aiPanelTitle)
        m_aiPanelTitle->setText(I18n::t("calendar.ai.panel_title"));
    if (m_aiHintLabel)
        m_aiHintLabel->setText(I18n::t("calendar.ai.hint"));
    if (m_aiCloseBtn)
        m_aiCloseBtn->setToolTip(I18n::t("common.close"));

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

        /* V4.2 #5 — narrow decorative pill */
        QLineEdit#AiCompactInput {
            background-color: %1;
            color: %3;
            border: 1px solid %2;
            border-radius: 19px;
            padding: 0 18px;
            font-size: 14px;
        }
        QLineEdit#AiCompactInput:hover {
            border: 1px solid %7;
            background-color: %6;
        }
        QPushButton#HistoryBtn {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 19px;
            color: %5;
            font-weight: 500;
            outline: 0;
        }
        QPushButton#HistoryBtn:hover { background-color: %6; color: %3; }

        /* V4.2 #5 — Spotlight-style popup card (drop shadow applied in code) */
        QWidget#AiExpandedPanel {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 14px;
        }
        QLabel#AiPanelTitle {
            color: %5;
            background: transparent;
            font-size: 13px;
            font-weight: 600;
            letter-spacing: 0.4px;
        }
        QLabel#AiHintLabel {
            color: %8;
            background: transparent;
            font-size: 13px;
        }
        QLabel#ParseStatusLabel {
            color: %5;
            background: transparent;
            font-size: 13px;
        }
        QLineEdit#AiFullInput {
            background-color: %4;
            color: %3;
            border: 1px solid %2;
            border-radius: 10px;
            padding: 0 14px;
            font-size: 15px;
        }
        QLineEdit#AiFullInput:focus { border: 1px solid %7; outline: 0; }
        QPushButton#AiCloseBtn {
            background-color: transparent;
            color: %5;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: 600;
            outline: 0;
        }
        QPushButton#AiCloseBtn:hover {
            background-color: %6;
            color: %3;
        }
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
        m_historyButton->setIcon(IconRenderer::icon(IconRenderer::History, t.textSecondary(), 16));
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
            // V4.3 #8 — 用 Preferences 的 weekStartOf 取代硬编码周一
            gridStart = Preferences::instance().weekStartOf(first);
            gridEnd   = gridStart.addDays(42);
            break;
        }
        case CalendarView::Week: {
            gridStart = Preferences::instance().weekStartOf(m_currentDate);
            gridEnd   = gridStart.addDays(7);
            break;
        }
        case CalendarView::Day: {
            // V4.2 #11: day view shows 2 days, so extend the range one more day
            gridStart = m_currentDate;
            gridEnd   = m_currentDate.addDays(2);
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
    // V4.4 #2 — 月视图点格子直接切到日视图（之前是弹新建事件对话框）。
    // 用户原话："月视图点击日的块应该直接切日视图。"
    // 新建事件保留：Ctrl+N、右上角"+ 新建日程"、日视图双击空白时间。
    m_currentDate = d;
    setView(CalendarView::Day);
}

void CalendarPage::onMonthOverflowClicked(const QDate &d, const QList<CalendarEvent> &events) {
    // V4.3 #9 — 之前点 "+N" 会弹一个 QDialog 列出当天所有事件，繁琐且和日视图重复。
    // 改成直接切到日视图并定位到点击的日期，更顺滑。
    Q_UNUSED(events);
    m_currentDate = d;
    setView(CalendarView::Day);
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
    // Collapse the panel after a successful import
    QTimer::singleShot(400, this, &CalendarPage::collapseAiPanel);
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
