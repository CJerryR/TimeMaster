#include "ChatPage.h"
#include "Theme.h"
#include "IconRenderer.h"
#include "../core/DeepSeekClient.h"
#include "../core/Database.h"
#include "../core/I18n.h"
#include "../utils/MarkdownToHtml.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QFrame>
#include <QSettings>

namespace timemaster {

namespace {
void bindMessageText(QLabel *bubble, const QString &text, bool isUser) {
    if (isUser) {
        bubble->setTextFormat(Qt::PlainText);
        bubble->setText(text);
    } else {
        QString html = MarkdownToHtml::convert(text);
        if (html.isEmpty()) html = "<span>…</span>";
        bubble->setTextFormat(Qt::RichText);
        bubble->setText(html);
    }
}
} // namespace

ChatPage::ChatPage(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QWidget(parent), m_db(db), m_ai(ai)
{
    setObjectName("ChatPage");

    // 持久化的隐私开关
    QSettings settings;
    m_aiSeesCalendar = settings.value("ai_sees_calendar", true).toBool();

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);

    // ---- Top bar ----
    auto *topBar = new QFrame;
    topBar->setObjectName("ChatTopBar");
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(16, 12, 12, 12);
    topLayout->setSpacing(12);

    m_titleIcon = new QLabel;
    m_titleIcon->setObjectName("ChatPageTitleIcon");
    m_titleIcon->setFixedSize(20, 20);

    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("ChatPageTitle");

    // Privacy chip — replaces the old isolated checkbox (V4 § 6.4)
    m_privacyChip = new QPushButton;
    m_privacyChip->setObjectName("PrivacyChip");
    m_privacyChip->setCursor(Qt::PointingHandCursor);
    m_privacyChip->setCheckable(false);
    connect(m_privacyChip, &QPushButton::clicked, this, &ChatPage::onPrivacyChipClicked);

    m_clearBtn = new QPushButton;
    m_clearBtn->setObjectName("ChatGhostBtn");
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, &ChatPage::onClear);

    topLayout->addWidget(m_titleIcon);
    topLayout->addWidget(m_titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_privacyChip);
    topLayout->addWidget(m_clearBtn);
    root->addWidget(topBar);

    // ---- Message scroll area ----
    auto *chatCard = new QFrame;
    chatCard->setObjectName("ChatCard");
    auto *chatCardLayout = new QVBoxLayout(chatCard);
    chatCardLayout->setContentsMargins(0, 0, 0, 0);

    m_scroll = new QScrollArea;
    m_scroll->setObjectName("ChatScroll");
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_msgContainer = new QWidget;
    m_msgContainer->setObjectName("ChatMsgContainer");
    m_msgLayout = new QVBoxLayout(m_msgContainer);
    m_msgLayout->setContentsMargins(38, 24, 38, 24);
    m_msgLayout->setSpacing(14);

    // Empty state (initial)
    m_emptyState = new QWidget;
    m_emptyState->setObjectName("ChatEmptyHost");
    auto *emptyLay = new QVBoxLayout(m_emptyState);
    emptyLay->setContentsMargins(0, 60, 0, 40);
    emptyLay->setSpacing(10);
    emptyLay->setAlignment(Qt::AlignCenter);

    m_emptyTitle = new QLabel;
    m_emptyTitle->setObjectName("ChatEmptyTitle");
    m_emptyTitle->setAlignment(Qt::AlignCenter);
    {
        QFont f; f.setPointSize(18); f.setWeight(QFont::DemiBold);
        m_emptyTitle->setFont(f);
    }
    emptyLay->addWidget(m_emptyTitle);

    m_emptySubtitle = new QLabel;
    m_emptySubtitle->setObjectName("ChatEmptySubtitle");
    m_emptySubtitle->setAlignment(Qt::AlignCenter);
    emptyLay->addWidget(m_emptySubtitle);

    emptyLay->addSpacing(12);

    // Suggestion bubbles (three)
    auto *bubblesRow = new QVBoxLayout;
    bubblesRow->setSpacing(8);
    bubblesRow->setAlignment(Qt::AlignHCenter);
    for (const QString &key : {QStringLiteral("chat.suggest.busy"),
                                QStringLiteral("chat.suggest.plan"),
                                QStringLiteral("chat.suggest.where")}) {
        auto *b = new QPushButton;
        b->setObjectName("ChatSuggestBubble");
        b->setCursor(Qt::PointingHandCursor);
        b->setProperty("i18nKey", key);
        connect(b, &QPushButton::clicked, this, [this, b]{
            sendCannedQuery(b->text());
        });
        bubblesRow->addWidget(b, 0, Qt::AlignHCenter);
        m_suggestionButtons.append(b);
    }
    emptyLay->addLayout(bubblesRow);

    m_msgLayout->addWidget(m_emptyState, 0, Qt::AlignCenter);
    m_msgLayout->addStretch();

    m_scroll->setWidget(m_msgContainer);
    chatCardLayout->addWidget(m_scroll);
    root->addWidget(chatCard, 1);

    // ---- Input ----
    auto *inputCard = new QFrame;
    inputCard->setObjectName("ChatInputCard");
    auto *inputLayout = new QHBoxLayout(inputCard);
    inputLayout->setContentsMargins(14, 10, 10, 10);
    inputLayout->setSpacing(10);

    m_input = new QLineEdit;
    m_input->setObjectName("ChatInput");
    m_input->setMinimumHeight(40);

    m_sendBtn = new QPushButton;
    m_sendBtn->setObjectName("ChatSendBtn");
    m_sendBtn->setMinimumHeight(40);
    m_sendBtn->setMinimumWidth(96);
    m_sendBtn->setCursor(Qt::PointingHandCursor);

    connect(m_input, &QLineEdit::returnPressed, this, &ChatPage::onSend);
    connect(m_sendBtn, &QPushButton::clicked,   this, &ChatPage::onSend);

    inputLayout->addWidget(m_input);
    inputLayout->addWidget(m_sendBtn);
    root->addWidget(inputCard);

    connect(m_ai, &DeepSeekClient::chatChunk,    this, &ChatPage::onChatChunk);
    connect(m_ai, &DeepSeekClient::chatFinished, this, &ChatPage::onChatFinished);
    connect(m_ai, &DeepSeekClient::chatError,    this, &ChatPage::onChatError);

    connect(&Theme::instance(), &Theme::changed,        this, &ChatPage::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, &ChatPage::applyLanguage);

    applyLanguage();
    applyTheme();
}

void ChatPage::onPrivacyChipClicked() {
    // Toggle in place; updates the chip text instantly
    setAiSeesCalendar(!m_aiSeesCalendar);
}

void ChatPage::setAiSeesCalendar(bool v) {
    m_aiSeesCalendar = v;
    QSettings().setValue("ai_sees_calendar", v);
    updatePrivacyChip();
}

void ChatPage::updatePrivacyChip() {
    if (!m_privacyChip) return;
    auto &t = Theme::instance();
    if (m_aiSeesCalendar) {
        m_privacyChip->setText(I18n::t("chat.ctx.chip_fmt"));
        QString brand14 = QString("rgba(%1,%2,%3,0.14)")
            .arg(t.brand().red()).arg(t.brand().green()).arg(t.brand().blue());
        QString brand32 = QString("rgba(%1,%2,%3,0.32)")
            .arg(t.brand().red()).arg(t.brand().green()).arg(t.brand().blue());
        m_privacyChip->setStyleSheet(QString(
            "QPushButton#PrivacyChip { background-color:%1; color:%2; "
            "border:1px solid %3; border-radius:6px; padding:5px 10px; "
            "font-size:12px; font-weight:600; }"
            "QPushButton#PrivacyChip:hover { background-color:%4; }")
            .arg(brand14).arg(t.brand().name()).arg(brand32).arg(brand14));
    } else {
        m_privacyChip->setText(I18n::t("chat.ctx.chip_off"));
        m_privacyChip->setStyleSheet(QString(
            "QPushButton#PrivacyChip { background-color:transparent; color:%1; "
            "border:1px solid %2; border-radius:6px; padding:5px 10px; "
            "font-size:12px; font-weight:500; }"
            "QPushButton#PrivacyChip:hover { background-color:%3; color:%4; }")
            .arg(t.textSecondary().name())
            .arg(t.strokeRgba())
            .arg(t.cardBgHoverRgba())
            .arg(t.textPrimary().name()));
    }
    m_privacyChip->setToolTip(I18n::t("chat.ctx.tip"));
}

void ChatPage::applyLanguage() {
    if (m_titleLabel)    m_titleLabel->setText(I18n::t("chat.title"));
    if (m_input)         m_input->setPlaceholderText(I18n::t("chat.placeholder"));
    if (m_sendBtn)       m_sendBtn->setText(I18n::t("chat.send") + QStringLiteral(" →"));
    if (m_clearBtn)      m_clearBtn->setText(I18n::t("chat.clear"));
    if (m_emptyTitle)    m_emptyTitle->setText(I18n::t("chat.empty.title"));
    if (m_emptySubtitle) m_emptySubtitle->setText(I18n::t("chat.empty.subtitle"));
    for (auto *b : m_suggestionButtons) {
        b->setText(I18n::t(b->property("i18nKey").toString()));
    }
    updatePrivacyChip();
}

QString ChatPage::basePrompt() const {
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd dddd");
    return I18n::t("chat.prompt.persona_en").arg(today);
}

QString ChatPage::buildCalendarContext() const {
    if (!m_db) return QString();

    QDateTime start(QDate::currentDate().addDays(-7), QTime(0, 0, 0));
    QDateTime end(QDate::currentDate().addDays(14),   QTime(23, 59, 59));
    auto events = m_db->getEventsByRange(start, end);

    bool en = I18n::instance().isEnglish();
    QString header = en
        ? "[User calendar] (past 7 days ~ next 14 days)\n"
        : "【用户当前日历】（过去 7 天 ~ 未来 14 天）\n";
    QString emptyLine = en
        ? "[User calendar]\n(No events in past 7 days ~ next 14 days.)"
        : "【用户当前日历】\n（过去 7 天 ~ 未来 14 天范围内暂无任何日程）";
    QString allDay = en ? "all day" : "全天";
    QString urgent = en ? " · urgent" : " · 紧急";

    if (events.isEmpty()) return emptyLine;

    QString lines = header;
    const int kMaxEvents = 80;
    int count = qMin(int(events.size()), kMaxEvents);

    for (int i = 0; i < count; ++i) {
        const auto &e = events[i];
        QString timeStr;
        if (e.allDay) {
            timeStr = e.startDate.toString("yyyy-MM-dd") + " " + allDay;
        } else if (e.startDate.date() == e.endDate.date()) {
            timeStr = QString("%1 %2-%3")
                .arg(e.startDate.toString("yyyy-MM-dd"))
                .arg(e.startDate.toString("HH:mm"))
                .arg(e.endDate.toString("HH:mm"));
        } else {
            timeStr = QString("%1 ~ %2")
                .arg(e.startDate.toString("yyyy-MM-dd HH:mm"))
                .arg(e.endDate.toString("MM-dd HH:mm"));
        }

        QString catLabel;
        switch (e.category) {
            case EventCategory::Work:          catLabel = I18n::t("cat.work"); break;
            case EventCategory::Study:         catLabel = I18n::t("cat.study"); break;
            case EventCategory::Entertainment: catLabel = I18n::t("cat.entertainment"); break;
            case EventCategory::Exercise:      catLabel = I18n::t("cat.exercise"); break;
            case EventCategory::Rest:          catLabel = I18n::t("cat.rest"); break;
            case EventCategory::Social:        catLabel = I18n::t("cat.social"); break;
            case EventCategory::Personal:      catLabel = I18n::t("cat.personal"); break;
            case EventCategory::Other:         catLabel = I18n::t("cat.other"); break;
        }

        QString line = QString("· %1 | %2").arg(timeStr, e.title);
        line += QString(" (%1").arg(catLabel);
        if (e.priority == EventPriority::Urgent) line += urgent;
        line += ")";
        if (!e.location.isEmpty()) line += QString(" @ %1").arg(e.location);
        lines += line + "\n";
    }
    if (events.size() > kMaxEvents) {
        lines += QString(en
            ? "(Showing the first %1 of %2 events.)\n"
            : "（共 %2 条，仅展示前 %1 条）\n").arg(kMaxEvents).arg(events.size());
    }
    return lines;
}

void ChatPage::sendCannedQuery(const QString &q) {
    if (m_isResponding) return;
    m_input->setText(q);
    onSend();
}

void ChatPage::onSend() {
    QString text = m_input->text().trimmed();
    if (text.isEmpty() || m_isResponding) return;

    if (!m_ai->hasApiKey()) {
        appendBubble(I18n::t("chat.api.missing"), false);
        return;
    }

    if (m_emptyState) m_emptyState->hide();

    appendBubble(text, true);
    m_input->clear();

    m_isResponding = true;
    m_sendBtn->setEnabled(false);
    m_streamingText.clear();
    m_currentStreamingBubble = appendBubble("…", false, true);

    QString ctx;
    if (m_aiSeesCalendar) {
        ctx = buildCalendarContext();
    }

    m_ai->sendChat(text, basePrompt(), ctx);
}

void ChatPage::onChatChunk(const QString &delta) {
    if (!m_currentStreamingBubble) return;
    m_streamingText += delta;
    bindMessageText(m_currentStreamingBubble, m_streamingText, false);
    scrollToBottom();
}

void ChatPage::onChatFinished(const QString &full) {
    QString text = full.isEmpty() ? m_streamingText : full;
    if (m_currentStreamingBubble) {
        bindMessageText(m_currentStreamingBubble, text, false);
        m_currentStreamingBubble = nullptr;
    }
    m_isResponding = false;
    m_sendBtn->setEnabled(true);
    scrollToBottom();
}

void ChatPage::onChatError(const QString &msg) {
    QString prefix = I18n::t("chat.error.prefix");
    if (m_currentStreamingBubble) {
        bindMessageText(m_currentStreamingBubble, prefix + msg, false);
        m_currentStreamingBubble = nullptr;
    } else {
        appendBubble(prefix + msg, false);
    }
    m_isResponding = false;
    m_sendBtn->setEnabled(true);
}

void ChatPage::rebuildEmptyState() {
    if (!m_emptyState) return;
    m_emptyTitle->setText(I18n::t("chat.empty.title"));
    m_emptySubtitle->setText(I18n::t("chat.empty.subtitle"));
    for (auto *b : m_suggestionButtons) {
        b->setText(I18n::t(b->property("i18nKey").toString()));
    }
    m_emptyState->show();
}

void ChatPage::onClear() {
    m_bubbles.clear();
    // Take everything off the layout except the empty state widget (re-added below)
    while (m_msgLayout->count() > 0) {
        auto *item = m_msgLayout->takeAt(0);
        if (item->widget() && item->widget() != m_emptyState) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    if (!m_emptyState->parent()) {
        // shouldn't happen, but rebuild defensively
        m_emptyState = nullptr;
    }
    if (m_emptyState) {
        m_msgLayout->addWidget(m_emptyState, 0, Qt::AlignCenter);
        m_emptyState->show();
    }
    m_msgLayout->addStretch();
    m_currentStreamingBubble = nullptr;
    m_isResponding = false;
    m_sendBtn->setEnabled(true);
    rebuildEmptyState();
    applyTheme();
}

QLabel *ChatPage::appendBubble(const QString &text, bool isUser, bool isStreaming) {
    Q_UNUSED(isStreaming);

    int stretchIdx = -1;
    for (int i = m_msgLayout->count() - 1; i >= 0; --i) {
        if (m_msgLayout->itemAt(i)->spacerItem()) { stretchIdx = i; break; }
    }
    if (stretchIdx >= 0) {
        auto *item = m_msgLayout->takeAt(stretchIdx);
        delete item;
    }

    auto *row = new QWidget;
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(0);

    auto *bubble = new QLabel;
    bubble->setWordWrap(true);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bubble->setObjectName(isUser ? "ChatBubbleUser" : "ChatBubbleAI");
    bindMessageText(bubble, text, isUser);

    m_bubbles.append(bubble);
    connect(bubble, &QObject::destroyed, this, [this, bubble]() {
        m_bubbles.removeAll(bubble);
    });

    if (isUser) {
        rowLayout->addStretch();
        rowLayout->addWidget(bubble);
    } else {
        rowLayout->addWidget(bubble);
        rowLayout->addStretch();
    }

    m_msgLayout->addWidget(row);
    m_msgLayout->addStretch();

    updateBubblesMaxWidth();
    QTimer::singleShot(0, this, &ChatPage::scrollToBottom);
    return bubble;
}

void ChatPage::scrollToBottom() {
    auto *bar = m_scroll->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void ChatPage::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    updateBubblesMaxWidth();
}

void ChatPage::updateBubblesMaxWidth() {
    if (!m_scroll) return;
    int vw = m_scroll->viewport()->width();
    int maxW = qMax(240, vw - 76 - 8);
    int cap = int(vw * 0.92);
    if (cap > 0 && maxW > cap) maxW = cap;
    for (QLabel *b : m_bubbles) {
        if (b) b->setMaximumWidth(maxW);
    }
}

void ChatPage::applyTheme() {
    auto &t = Theme::instance();
    QString brand = t.brand().name();
    QString brandHover = t.brand().darker(110).name();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();
    QString strokeR = t.strokeRgba();
    QString cardBg = t.cardBgRgba();
    QString componentBg = t.componentBgRgba();
    QString hoverBg = t.cardBgHoverRgba();

    if (m_titleIcon) {
        m_titleIcon->setPixmap(IconRenderer::pixmap(IconRenderer::NavChat, t.brand(), 20));
    }

    setStyleSheet(QString(R"(
        QWidget#ChatPage { background: transparent; }

        QFrame#ChatTopBar {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }
        QLabel#ChatPageTitle {
            font-size: 16px;
            font-weight: 600;
            color: %3;
            letter-spacing: -0.1px;
        }
        QPushButton#ChatGhostBtn {
            background-color: transparent;
            color: %5;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 5px 12px;
            font-weight: 500;
        }
        QPushButton#ChatGhostBtn:hover { background-color: %6; color: %3; }

        QFrame#ChatCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }
        QScrollArea#ChatScroll { background-color: transparent; border: none; }
        QWidget#ChatMsgContainer { background-color: transparent; }
        QWidget#ChatEmptyHost { background: transparent; }

        QLabel#ChatEmptyTitle {
            color: %3;
            background: transparent;
            letter-spacing: -0.2px;
        }
        QLabel#ChatEmptySubtitle {
            color: %5;
            background: transparent;
            font-size: 14px;
        }
        QPushButton#ChatSuggestBubble {
            background-color: %4;
            color: %3;
            border: 1px solid %2;
            border-radius: 16px;
            padding: 9px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton#ChatSuggestBubble:hover {
            background-color: %6;
            border-color: %5;
        }

        QLabel#ChatBubbleUser {
            background-color: %7;
            color: white;
            border-radius: 14px;
            padding: 11px 16px;
            font-size: 14px;
        }
        QLabel#ChatBubbleAI {
            background-color: %4;
            color: %3;
            border: 1px solid %2;
            border-radius: 14px;
            padding: 12px 16px;
            font-size: 14px;
        }

        QFrame#ChatInputCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }
        QLineEdit#ChatInput {
            background-color: transparent;
            color: %3;
            border: none;
            border-radius: 8px;
            padding: 0 8px;
            font-size: 14px;
        }
        QLineEdit#ChatInput:focus { border: none; }
        QPushButton#ChatSendBtn {
            background-color: %7;
            color: white;
            border: none;
            border-radius: 8px;
            font-weight: 600;
            padding: 0 18px;
        }
        QPushButton#ChatSendBtn:hover { background-color: %8; }
        QPushButton#ChatSendBtn:disabled { background-color: %4; color: %5; }
    )")
    /*1*/.arg(cardBg)
    /*2*/.arg(strokeR)
    /*3*/.arg(textPrim)
    /*4*/.arg(componentBg)
    /*5*/.arg(textSec)
    /*6*/.arg(hoverBg)
    /*7*/.arg(brand)
    /*8*/.arg(brandHover));

    updatePrivacyChip();
}

} // namespace timemaster
