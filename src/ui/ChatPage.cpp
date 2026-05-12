#include "ChatPage.h"
#include "Theme.h"
#include "IconRenderer.h"
#include "../core/DeepSeekClient.h"
#include "../core/Database.h"
#include "../core/I18n.h"
#include "../core/Preferences.h"
#include "../utils/MarkdownToHtml.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QFrame>
#include <QSettings>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

namespace timemaster {

namespace {
void bindMessageText(QLabel *bubble, const QString &text, bool isUser) {
    if (isUser) {
        bubble->setTextFormat(Qt::PlainText);
        bubble->setText(text);
    } else {
        // V4.3 #7 — 渲染前先把 ```action ...``` 代码块剥掉（这些是给应用看的指令，
        // 不是给用户看的散文），然后再走 markdown 转 HTML。否则用户会看到一堆
        // 难看的 JSON 残骸。
        QString cleaned = text;
        static const QRegularExpression actionBlock(
            R"(```action\s*\n?(.*?)\n?```)",
            QRegularExpression::DotMatchesEverythingOption);
        cleaned.remove(actionBlock);
        QString html = MarkdownToHtml::convert(cleaned.trimmed());
        if (html.isEmpty()) html = "<span>…</span>";
        bubble->setTextFormat(Qt::RichText);
        bubble->setText(html);
    }
}

QString opLabelI18nKey(const QString &op) {
    if (op == "add")    return QStringLiteral("chat.action.op_add");
    if (op == "delete") return QStringLiteral("chat.action.op_delete");
    if (op == "update") return QStringLiteral("chat.action.op_update");
    return QStringLiteral("chat.action.op_unknown");
}

EventColor colorFromString(const QString &s) {
    QString k = s.trimmed().toLower();
    if (k == "red")      return EventColor::Red;
    if (k == "orange")   return EventColor::Orange;
    if (k == "yellow")   return EventColor::Yellow;
    if (k == "green")    return EventColor::Green;
    if (k == "blue")     return EventColor::Blue;
    if (k == "purple")   return EventColor::Purple;
    if (k == "pink")     return EventColor::Pink;
    if (k == "brown")    return EventColor::Brown;
    if (k == "gray" || k == "grey") return EventColor::Gray;
    if (k == "cyan")     return EventColor::Cyan;
    if (k == "indigo")   return EventColor::Indigo;
    if (k == "teal")     return EventColor::Teal;
    return EventColor::Blue;
}

EventCategory categoryFromString(const QString &s) {
    QString k = s.trimmed().toLower();
    if (k == "work")          return EventCategory::Work;
    if (k == "study")         return EventCategory::Study;
    if (k == "entertainment") return EventCategory::Entertainment;
    if (k == "exercise")      return EventCategory::Exercise;
    if (k == "rest")          return EventCategory::Rest;
    if (k == "social")        return EventCategory::Social;
    if (k == "personal")      return EventCategory::Personal;
    return EventCategory::Other;
}

// 把一个 CalendarEvent 序列化为 JSON 字符串，供 ChatActions.snapshot_json 持久化用。
QString eventToJson(const CalendarEvent &e) {
    QJsonObject o;
    o["id"]          = e.id;
    o["title"]       = e.title;
    o["description"] = e.description;
    o["start"]       = e.startDate.toString(Qt::ISODate);
    o["end"]         = e.endDate.toString(Qt::ISODate);
    o["all_day"]     = e.allDay;
    o["color"]       = colorToString(e.color);
    o["category"]    = categoryToString(e.category);
    o["location"]    = e.location;
    o["reminder"]    = e.reminder;
    o["priority"]    = priorityToString(e.priority);
    o["source"]      = sourceToString(e.source);
    o["ai_batch_id"] = e.aiBatchId;
    o["created_at"]  = e.createdAt.toString(Qt::ISODate);
    o["updated_at"]  = e.updatedAt.toString(Qt::ISODate);
    return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
}

CalendarEvent eventFromJson(const QString &json) {
    CalendarEvent e;
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    auto o = doc.object();
    e.id          = o.value("id").toString();
    e.title       = o.value("title").toString();
    e.description = o.value("description").toString();
    e.startDate   = QDateTime::fromString(o.value("start").toString(), Qt::ISODate);
    e.endDate     = QDateTime::fromString(o.value("end").toString(),   Qt::ISODate);
    e.allDay      = o.value("all_day").toBool();
    e.color       = colorFromString(o.value("color").toString());
    e.category    = categoryFromString(o.value("category").toString());
    e.location    = o.value("location").toString();
    e.reminder    = o.value("reminder").toInt(15);
    QString prio  = o.value("priority").toString().toLower();
    e.priority    = (prio == "urgent") ? EventPriority::Urgent
                                       : (prio == "normal") ? EventPriority::Normal
                                                            : EventPriority::Low;
    QString src   = o.value("source").toString().toLower();
    e.source      = (src == "manual")  ? EventSource::Manual
                  : (src == "aiparse") ? EventSource::AiParse
                                       : EventSource::Chat;
    e.aiBatchId   = o.value("ai_batch_id").toString();
    e.createdAt   = QDateTime::fromString(o.value("created_at").toString(), Qt::ISODate);
    e.updatedAt   = QDateTime::fromString(o.value("updated_at").toString(), Qt::ISODate);
    return e;
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

    // V4.2 #10 — replace the inline 7+14 privacy chip with a small settings
    // shortcut. The actual past/future days configuration now lives in
    // SettingsDialog. Clicking the chip opens settings.
    m_privacyChip = new QPushButton;
    m_privacyChip->setObjectName("PrivacyChip");
    m_privacyChip->setCursor(Qt::PointingHandCursor);
    m_privacyChip->setCheckable(false);
    m_privacyChip->setFocusPolicy(Qt::NoFocus);
    connect(m_privacyChip, &QPushButton::clicked, this, &ChatPage::onPrivacyChipClicked);

    m_clearBtn = new QPushButton;
    m_clearBtn->setObjectName("ChatGhostBtn");
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, &ChatPage::onClear);

    // V4.3 #7 — 操作历史按钮。点开切换抽屉显示，抽屉里两列展示最近被允许的
    // add / delete 操作，每行有一个撤销按钮。
    m_historyBtn = new QPushButton;
    m_historyBtn->setObjectName("ChatGhostBtn");
    m_historyBtn->setCursor(Qt::PointingHandCursor);
    m_historyBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_historyBtn, &QPushButton::clicked, this, &ChatPage::onHistoryDrawerToggled);

    topLayout->addWidget(m_titleIcon);
    topLayout->addWidget(m_titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_privacyChip);
    topLayout->addWidget(m_historyBtn);
    topLayout->addWidget(m_clearBtn);
    root->addWidget(topBar);

    // ---- V4.3 #7 操作历史抽屉（折叠态默认隐藏）----
    m_historyDrawer = new QFrame;
    m_historyDrawer->setObjectName("ChatActionDrawer");
    m_historyDrawer->setVisible(false);
    auto *drawerLayout = new QHBoxLayout(m_historyDrawer);
    drawerLayout->setContentsMargins(18, 14, 18, 14);
    drawerLayout->setSpacing(18);

    auto buildColumn = [](const QString &headerObj, QLabel *&headerLbl,
                          QVBoxLayout *&listLay) {
        auto *col = new QWidget;
        auto *colLay = new QVBoxLayout(col);
        colLay->setContentsMargins(0, 0, 0, 0);
        colLay->setSpacing(8);
        headerLbl = new QLabel;
        headerLbl->setObjectName(headerObj);
        colLay->addWidget(headerLbl);
        auto *listHost = new QWidget;
        listLay = new QVBoxLayout(listHost);
        listLay->setContentsMargins(0, 0, 0, 0);
        listLay->setSpacing(6);
        listLay->addStretch();
        colLay->addWidget(listHost);
        return col;
    };

    drawerLayout->addWidget(buildColumn("ChatActionColHeaderAdd",
                                        m_drawerAddedHeader,
                                        m_drawerAddedColumn), 1);
    drawerLayout->addWidget(buildColumn("ChatActionColHeaderDel",
                                        m_drawerDeletedHeader,
                                        m_drawerDeletedColumn), 1);

    root->addWidget(m_historyDrawer);

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

void ChatPage::refreshFromSettings() {
    // V4.2 #10 — refresh the chip text/enabled state from QSettings.
    updatePrivacyChip();
}

void ChatPage::onPrivacyChipClicked() {
    // V4.2 #10: chip is now a settings shortcut, not a toggle. Past/future
    // days and the enable flag all live in SettingsDialog.
    emit openSettingsRequested();
}

void ChatPage::setAiSeesCalendar(bool v) {
    m_aiSeesCalendar = v;
    QSettings().setValue("ai_sees_calendar", v);
    updatePrivacyChip();
}

void ChatPage::updatePrivacyChip() {
    if (!m_privacyChip) return;
    auto &t = Theme::instance();
    // V4.2 #10: always re-read from settings so changes in SettingsDialog
    // propagate the next time we re-style.
    QSettings s;
    m_aiSeesCalendar  = s.value("ai_sees_calendar", true).toBool();
    int past   = s.value("ai_context_past_days",   7).toInt();
    int future = s.value("ai_context_future_days", 14).toInt();

    if (m_aiSeesCalendar) {
        m_privacyChip->setText(I18n::t("chat.ctx.chip_fmt_v2").arg(past).arg(future));
        QString brand14 = QString("rgba(%1,%2,%3,0.14)")
            .arg(t.brand().red()).arg(t.brand().green()).arg(t.brand().blue());
        QString brand32 = QString("rgba(%1,%2,%3,0.32)")
            .arg(t.brand().red()).arg(t.brand().green()).arg(t.brand().blue());
        m_privacyChip->setStyleSheet(QString(
            "QPushButton#PrivacyChip { background-color:%1; color:%2; "
            "border:1px solid %3; border-radius:6px; padding:5px 12px; "
            "font-size:13px; font-weight:600; outline:0; }"
            "QPushButton#PrivacyChip:hover { background-color:%4; }")
            .arg(brand14).arg(t.brand().name()).arg(brand32).arg(brand14));
    } else {
        m_privacyChip->setText(I18n::t("chat.ctx.chip_off"));
        m_privacyChip->setStyleSheet(QString(
            "QPushButton#PrivacyChip { background-color:transparent; color:%1; "
            "border:1px dashed %2; border-radius:6px; padding:5px 12px; "
            "font-size:13px; font-weight:500; outline:0; }"
            "QPushButton#PrivacyChip:hover { background-color:%3; color:%4; }")
            .arg(t.textSecondary().name())
            .arg(t.strokeRgba())
            .arg(t.cardBgHoverRgba())
            .arg(t.textPrimary().name()));
    }
    m_privacyChip->setToolTip(I18n::t("chat.ctx.tip_v2"));
}

void ChatPage::applyLanguage() {
    if (m_titleLabel)    m_titleLabel->setText(I18n::t("chat.title"));
    if (m_input)         m_input->setPlaceholderText(I18n::t("chat.placeholder"));
    if (m_sendBtn)       m_sendBtn->setText(I18n::t("chat.send") + QStringLiteral(" →"));
    if (m_clearBtn)      m_clearBtn->setText(I18n::t("chat.clear"));
    if (m_historyBtn)    m_historyBtn->setText(I18n::t("chat.history.title"));
    if (m_drawerAddedHeader)   m_drawerAddedHeader->setText(I18n::t("chat.history.added"));
    if (m_drawerDeletedHeader) m_drawerDeletedHeader->setText(I18n::t("chat.history.deleted"));
    if (m_emptyTitle)    m_emptyTitle->setText(I18n::t("chat.empty.title"));
    if (m_emptySubtitle) m_emptySubtitle->setText(I18n::t("chat.empty.subtitle"));
    for (auto *b : m_suggestionButtons) {
        b->setText(I18n::t(b->property("i18nKey").toString()));
    }
    updatePrivacyChip();
}

QString ChatPage::basePrompt() const {
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd dddd");
    QString persona = I18n::t("chat.prompt.persona_en").arg(today);

    // V4.3 #7 — 教 AI 一种简易协议：如果它想增/删/改用户日程，应该在普通
    // 回复中插入 ```action {...}``` 代码块，每个块描述一个操作。前端会把
    // 这些块抽出来渲染成审批卡，用户点"允许"才会真正写库。
    bool en = I18n::instance().isEnglish();
    QString protocolEn = QStringLiteral(
        "\n\n--- CALENDAR ACTION PROTOCOL ---\n"
        "When the user clearly asks you to ADD, DELETE, or MODIFY events in their calendar, "
        "embed one or more action blocks in your reply, exactly like this:\n"
        "```action\n"
        "{\"op\":\"add\",\"title\":\"Project review\",\"start\":\"2026-05-14T14:00:00\","
        "\"end\":\"2026-05-14T15:00:00\",\"category\":\"work\",\"color\":\"blue\","
        "\"location\":\"\",\"reminder\":15,\"priority\":\"normal\"}\n"
        "```\n"
        "For delete, use {\"op\":\"delete\",\"event_id\":\"<id>\",\"title\":\"...\"}.\n"
        "For update, use {\"op\":\"update\",\"event_id\":\"<id>\", ...other fields...}.\n"
        "RULES:\n"
        "1. Each block contains exactly one JSON object.\n"
        "2. Dates use ISO 8601 local time (no Z suffix).\n"
        "3. event_id for delete/update MUST come from the calendar context provided.\n"
        "4. Do NOT emit action blocks for hypothetical or example schedules — only for "
        "operations the user is actually requesting.\n"
        "5. You should still write a short natural-language reply around the action block.\n"
        "6. The user will approve or deny each action individually; don't assume execution.\n"
    );
    QString protocolZh = QStringLiteral(
        "\n\n--- 日历操作协议 ---\n"
        "当用户明确要求你新增、删除或修改其日历事件时，请在普通回复中嵌入一个或多个 action 代码块，"
        "格式如下：\n"
        "```action\n"
        "{\"op\":\"add\",\"title\":\"项目评审\",\"start\":\"2026-05-14T14:00:00\","
        "\"end\":\"2026-05-14T15:00:00\",\"category\":\"work\",\"color\":\"blue\","
        "\"location\":\"\",\"reminder\":15,\"priority\":\"normal\"}\n"
        "```\n"
        "删除操作使用 {\"op\":\"delete\",\"event_id\":\"<id>\",\"title\":\"...\"}。\n"
        "修改操作使用 {\"op\":\"update\",\"event_id\":\"<id>\", ...要修改的字段...}。\n"
        "规则：\n"
        "1. 每个 action 块仅包含一个 JSON 对象。\n"
        "2. 时间使用 ISO 8601 本地时间（不要加 Z）。\n"
        "3. 删除/修改的 event_id 必须来自上下文中的日历列表。\n"
        "4. 不要对假设性或举例性的日程输出 action 块——只在用户真的要求执行操作时输出。\n"
        "5. action 块之外仍然要写一段自然语言回复。\n"
        "6. 每个 action 都需要用户单独确认，请勿假设已执行。\n"
    );

    return persona + (en ? protocolEn : protocolZh);
}

QString ChatPage::buildCalendarContext() const {
    if (!m_db) return QString();

    // V4.2 #10 — past/future days now come from settings
    QSettings s;
    int pastDays   = s.value("ai_context_past_days",   7).toInt();
    int futureDays = s.value("ai_context_future_days", 14).toInt();
    pastDays   = qBound(0, pastDays,   60);
    futureDays = qBound(0, futureDays, 60);

    QDateTime start(QDate::currentDate().addDays(-pastDays), QTime(0, 0, 0));
    QDateTime end(QDate::currentDate().addDays(futureDays),   QTime(23, 59, 59));
    auto events = m_db->getEventsByRange(start, end);

    bool en = I18n::instance().isEnglish();
    QString header = en
        ? QString("[User calendar] (past %1 day(s) ~ next %2 day(s))\n").arg(pastDays).arg(futureDays)
        : QString("【用户当前日历】（过去 %1 天 ~ 未来 %2 天）\n").arg(pastDays).arg(futureDays);
    QString emptyLine = en
        ? QString("[User calendar]\n(No events in past %1 day(s) ~ next %2 day(s). "
                  "Do NOT fabricate events that are not in this list.)").arg(pastDays).arg(futureDays)
        : QString("【用户当前日历】\n（过去 %1 天 ~ 未来 %2 天范围内暂无任何日程。"
                  "请勿编造列表之外的任何日程。）").arg(pastDays).arg(futureDays);
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
    } else {
        // V4.2 #10 — explicit anti-hallucination guard. Without this the model
        // tends to fabricate schedule details when the user asks calendar-shaped
        // questions like "what do I have tomorrow".
        bool en = I18n::instance().isEnglish();
        ctx = en
            ? QStringLiteral("[User calendar]\n"
                             "(Calendar access is disabled. You do NOT know the "
                             "user's schedule. If they ask about their schedule, "
                             "say you can't see it and that they can re-enable "
                             "calendar context in Settings. Do not invent any "
                             "events, times, or appointments.)")
            : QStringLiteral("【用户当前日历】\n"
                             "（用户已关闭日历可见性。你不知道用户的任何具体日程。"
                             "如果用户询问日程相关问题，请直接告知无法看到，"
                             "并提示可以在「设置」中重新开启。"
                             "禁止编造任何具体日程、时间或事件。）");
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
    // V4.3 #7 — 流式结束后解析 action 块，渲染审批卡。注意只在 finished 时解析，
    // 不在 chunk 里做，避免半截 JSON 导致漏卡或重复卡。
    parseAndRenderActions(text);
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

void ChatPage::onHistoryDrawerToggled() {
    m_drawerOpen = !m_drawerOpen;
    m_historyDrawer->setVisible(m_drawerOpen);
    if (m_drawerOpen) reloadActionDrawer();
}

void ChatPage::reloadActionDrawer() {
    if (!m_drawerAddedColumn || !m_drawerDeletedColumn) return;

    // 清空两列（保留最后的 stretch）
    auto clearCol = [](QVBoxLayout *col) {
        // 留最后的 stretch (spacerItem)，其他全部 deleteLater
        while (col->count() > 1) {
            QLayoutItem *it = col->takeAt(0);
            if (it->widget()) it->widget()->deleteLater();
            delete it;
        }
    };
    clearCol(m_drawerAddedColumn);
    clearCol(m_drawerDeletedColumn);

    auto actions = m_db->getRecentChatActions(50);
    auto &t = Theme::instance();

    int addedCount = 0, delCount = 0;
    for (const auto &a : actions) {
        if (a.op != "add" && a.op != "delete" && a.op != "update") continue;

        auto *row = new QFrame;
        row->setObjectName("ChatActionHistoryRow");
        auto *rowLay = new QHBoxLayout(row);
        rowLay->setContentsMargins(10, 8, 10, 8);
        rowLay->setSpacing(8);

        auto *summary = new QLabel(a.humanSummary);
        summary->setObjectName("ChatActionHistoryText");
        summary->setWordWrap(true);
        summary->setStyleSheet(QString("color:%1;font-size:13px;")
                                .arg(t.textPrimary().name()));
        rowLay->addWidget(summary, 1);

        auto *undoBtn = new QPushButton(I18n::t("chat.action.undo"));
        undoBtn->setObjectName("ChatActionUndoBtn");
        undoBtn->setCursor(Qt::PointingHandCursor);
        undoBtn->setFocusPolicy(Qt::NoFocus);
        undoBtn->setStyleSheet(QString(
            "QPushButton{background:transparent;color:%1;border:1px solid %2;"
            "border-radius:6px;padding:3px 10px;font-size:12px;outline:0;}"
            "QPushButton:hover{background:%3;color:%4;}")
            .arg(t.textSecondary().name())
            .arg(t.strokeRgba())
            .arg(t.cardBgHoverRgba())
            .arg(t.textPrimary().name()));
        ChatAction captured = a;
        connect(undoBtn, &QPushButton::clicked, this, [this, captured]{
            undoAction(captured);
        });
        rowLay->addWidget(undoBtn);

        row->setStyleSheet(QString(
            "QFrame#ChatActionHistoryRow{background:%1;border:1px solid %2;"
            "border-radius:8px;}")
            .arg(t.componentBgRgba())
            .arg(t.strokeRgba()));

        if (a.op == "delete") {
            m_drawerDeletedColumn->insertWidget(m_drawerDeletedColumn->count() - 1, row);
            ++delCount;
        } else {
            // add 和 update 都进 "Added/Modified" 这一列
            m_drawerAddedColumn->insertWidget(m_drawerAddedColumn->count() - 1, row);
            ++addedCount;
        }
    }

    if (addedCount == 0) {
        auto *empty = new QLabel(I18n::t("chat.history.empty"));
        empty->setObjectName("ChatActionHistoryEmpty");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet(QString("color:%1;font-size:13px;padding:14px;")
                              .arg(t.textPlaceholder().name()));
        m_drawerAddedColumn->insertWidget(m_drawerAddedColumn->count() - 1, empty);
    }
    if (delCount == 0) {
        auto *empty = new QLabel(I18n::t("chat.history.empty"));
        empty->setObjectName("ChatActionHistoryEmpty");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet(QString("color:%1;font-size:13px;padding:14px;")
                              .arg(t.textPlaceholder().name()));
        m_drawerDeletedColumn->insertWidget(m_drawerDeletedColumn->count() - 1, empty);
    }
}

void ChatPage::parseAndRenderActions(const QString &fullAiText) {
    // V4.3 #7 — 提取所有 ```action ... ``` 块。每块一个 JSON 对象，
    // 描述一次日历操作。每个被成功解析的块都会渲染一张审批卡。
    static const QRegularExpression re(
        R"(```action\s*\n?(.*?)\n?```)",
        QRegularExpression::DotMatchesEverythingOption);

    auto matches = re.globalMatch(fullAiText);
    while (matches.hasNext()) {
        auto m = matches.next();
        QString jsonText = m.captured(1).trimmed();
        if (jsonText.isEmpty()) continue;

        QJsonParseError err{};
        auto doc = QJsonDocument::fromJson(jsonText.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "ChatPage: action block parse failed:" << err.errorString();
            continue;
        }
        QJsonObject obj = doc.object();
        QString op = obj.value("op").toString().toLower();
        if (op != "add" && op != "delete" && op != "update") continue;

        // 构造 snapshot：add 时是新事件，delete/update 时尽量带上原事件以便撤销
        QString eventId = obj.value("event_id").toString();
        if (eventId.isEmpty()) eventId = obj.value("id").toString();

        CalendarEvent ev;
        if (op == "add") {
            ev.id          = Database::generateId();
            ev.title       = obj.value("title").toString();
            ev.description = obj.value("description").toString();
            ev.startDate   = QDateTime::fromString(obj.value("start").toString(), Qt::ISODate);
            if (!ev.startDate.isValid()) ev.startDate = QDateTime::currentDateTime();
            ev.endDate     = QDateTime::fromString(obj.value("end").toString(), Qt::ISODate);
            if (!ev.endDate.isValid())   ev.endDate = ev.startDate.addSecs(3600);
            ev.allDay      = obj.value("all_day").toBool(false);
            ev.color       = colorFromString(obj.value("color").toString("blue"));
            ev.category    = categoryFromString(obj.value("category").toString("work"));
            ev.location    = obj.value("location").toString();
            ev.reminder    = obj.value("reminder").toInt(15);
            QString prio   = obj.value("priority").toString("normal").toLower();
            ev.priority    = (prio == "urgent") ? EventPriority::Urgent
                                                : (prio == "low")    ? EventPriority::Low
                                                                     : EventPriority::Normal;
            ev.source      = EventSource::Chat;
            ev.createdAt   = QDateTime::currentDateTime();
            ev.updatedAt   = ev.createdAt;
            eventId = ev.id;
        } else {
            // delete / update：先尝试加载原事件作为 snapshot
            auto orig = m_db->getEventById(eventId);
            if (orig.has_value()) {
                ev = *orig;
            } else {
                ev.id    = eventId;
                ev.title = obj.value("title").toString();
            }
            if (op == "update") {
                // 把 JSON 中提到的字段覆盖到 ev
                if (obj.contains("title"))       ev.title = obj.value("title").toString();
                if (obj.contains("description")) ev.description = obj.value("description").toString();
                if (obj.contains("start"))       ev.startDate = QDateTime::fromString(obj.value("start").toString(), Qt::ISODate);
                if (obj.contains("end"))         ev.endDate   = QDateTime::fromString(obj.value("end").toString(),   Qt::ISODate);
                if (obj.contains("all_day"))     ev.allDay    = obj.value("all_day").toBool();
                if (obj.contains("color"))       ev.color     = colorFromString(obj.value("color").toString());
                if (obj.contains("category"))    ev.category  = categoryFromString(obj.value("category").toString());
                if (obj.contains("location"))    ev.location  = obj.value("location").toString();
                if (obj.contains("reminder"))    ev.reminder  = obj.value("reminder").toInt(15);
                if (obj.contains("priority")) {
                    QString p = obj.value("priority").toString().toLower();
                    ev.priority = (p == "urgent") ? EventPriority::Urgent
                                                  : (p == "low")    ? EventPriority::Low
                                                                    : EventPriority::Normal;
                }
                ev.updatedAt = QDateTime::currentDateTime();
            }
        }

        // 构造人类可读的摘要
        QString summary;
        bool en = I18n::instance().isEnglish();
        QString timeStr = ev.startDate.isValid()
            ? ev.startDate.toString("MM-dd HH:mm")
            : QString("?");
        if (op == "add") {
            summary = en ? QString("Add · %1 · %2").arg(ev.title, timeStr)
                         : QString("新增 · %1 · %2").arg(ev.title, timeStr);
        } else if (op == "delete") {
            summary = en ? QString("Delete · %1").arg(ev.title.isEmpty() ? eventId : ev.title)
                         : QString("删除 · %1").arg(ev.title.isEmpty() ? eventId : ev.title);
        } else {
            summary = en ? QString("Update · %1 · %2").arg(ev.title, timeStr)
                         : QString("修改 · %1 · %2").arg(ev.title, timeStr);
        }

        QString snapshot = eventToJson(ev);

        // 检查自动审批
        bool autoOk = false;
        if (op == "add"    && Preferences::instance().autoApproveAdd())    autoOk = true;
        if (op == "delete" && Preferences::instance().autoApproveDelete()) autoOk = true;
        if (op == "update" && Preferences::instance().autoApproveUpdate()) autoOk = true;

        if (autoOk) {
            executeAction(op, snapshot, summary, eventId);
        } else {
            appendActionCard(op, summary, snapshot, eventId);
        }
    }
}

void ChatPage::appendActionCard(const QString &op, const QString &humanSummary,
                                const QString &snapshotJson, const QString &eventId) {
    auto &t = Theme::instance();

    // 取出末尾 stretch
    int stretchIdx = -1;
    for (int i = m_msgLayout->count() - 1; i >= 0; --i) {
        if (m_msgLayout->itemAt(i)->spacerItem()) { stretchIdx = i; break; }
    }
    if (stretchIdx >= 0) {
        delete m_msgLayout->takeAt(stretchIdx);
    }

    auto *card = new QFrame;
    card->setObjectName("ChatActionCard");
    card->setStyleSheet(QString(
        "QFrame#ChatActionCard{background:%1;border:1px solid %2;border-radius:12px;}")
        .arg(t.cardBgRgba())
        .arg(t.brand().name()));

    auto *outerLay = new QVBoxLayout(card);
    outerLay->setContentsMargins(16, 14, 16, 14);
    outerLay->setSpacing(10);

    // header row
    auto *headerRow = new QHBoxLayout;
    headerRow->setSpacing(8);
    auto *badge = new QLabel(I18n::t(opLabelI18nKey(op)));
    badge->setStyleSheet(QString(
        "background:%1;color:white;border-radius:8px;padding:3px 10px;"
        "font-size:12px;font-weight:700;")
        .arg(t.brand().name()));
    headerRow->addWidget(badge);
    auto *summaryLbl = new QLabel(humanSummary);
    summaryLbl->setWordWrap(true);
    summaryLbl->setStyleSheet(QString("color:%1;font-size:14px;font-weight:600;")
                               .arg(t.textPrimary().name()));
    headerRow->addWidget(summaryLbl, 1);
    outerLay->addLayout(headerRow);

    // detail (compact JSON preview)
    auto *detail = new QLabel(snapshotJson);
    detail->setWordWrap(true);
    detail->setStyleSheet(QString(
        "background:%1;color:%2;border-radius:8px;padding:8px 10px;"
        "font-family:monospace;font-size:12px;")
        .arg(t.componentBgRgba())
        .arg(t.textSecondary().name()));
    outerLay->addWidget(detail);

    // action buttons row
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    btnRow->addStretch();

    auto *denyBtn   = new QPushButton(I18n::t("chat.action.deny"));
    auto *onceBtn   = new QPushButton(I18n::t("chat.action.allow_once"));
    auto *alwaysBtn = new QPushButton(I18n::t("chat.action.always_allow"));
    for (auto *b : {denyBtn, onceBtn, alwaysBtn}) {
        b->setCursor(Qt::PointingHandCursor);
        b->setFocusPolicy(Qt::NoFocus);
        b->setMinimumHeight(34);
    }
    denyBtn->setStyleSheet(QString(
        "QPushButton{background:transparent;color:%1;border:1px solid %2;"
        "border-radius:8px;padding:6px 14px;font-size:13px;outline:0;}"
        "QPushButton:hover{background:%3;color:%4;}")
        .arg(t.textSecondary().name())
        .arg(t.strokeRgba())
        .arg(t.cardBgHoverRgba())
        .arg(t.textPrimary().name()));
    onceBtn->setStyleSheet(QString(
        "QPushButton{background:transparent;color:%1;border:1px solid %1;"
        "border-radius:8px;padding:6px 14px;font-size:13px;font-weight:600;outline:0;}"
        "QPushButton:hover{background:rgba(%2,%3,%4,30);}")
        .arg(t.brand().name())
        .arg(t.brand().red()).arg(t.brand().green()).arg(t.brand().blue()));
    alwaysBtn->setStyleSheet(QString(
        "QPushButton{background:%1;color:white;border:none;border-radius:8px;"
        "padding:6px 14px;font-size:13px;font-weight:600;outline:0;}"
        "QPushButton:hover{background:%2;}")
        .arg(t.brand().name())
        .arg(t.brand().darker(110).name()));

    btnRow->addWidget(denyBtn);
    btnRow->addWidget(onceBtn);
    btnRow->addWidget(alwaysBtn);
    outerLay->addLayout(btnRow);

    m_msgLayout->addWidget(card);
    m_msgLayout->addStretch();

    // 关掉所有按钮 + 把卡变成 "已执行 / 已拒绝" 提示的 helper
    auto markResolved = [card, &t, this](const QString &resolutionText, bool ok) {
        // 移除所有子 widget 替换成一行 resolution
        QLayoutItem *it;
        while ((it = card->layout()->takeAt(0))) {
            if (it->widget()) it->widget()->deleteLater();
            if (auto *sub = it->layout()) {
                QLayoutItem *si;
                while ((si = sub->takeAt(0))) {
                    if (si->widget()) si->widget()->deleteLater();
                    delete si;
                }
                delete sub;
            } else {
                delete it;
            }
        }
        auto *line = new QLabel(resolutionText);
        line->setStyleSheet(QString("color:%1;font-size:13px;font-weight:600;")
                              .arg(ok ? t.brand().name() : t.textSecondary().name()));
        card->layout()->addWidget(line);
        card->setStyleSheet(QString(
            "QFrame#ChatActionCard{background:%1;border:1px solid %2;border-radius:12px;}")
            .arg(t.cardBgRgba())
            .arg(t.strokeRgba()));
    };

    connect(denyBtn, &QPushButton::clicked, this, [markResolved, this]{
        markResolved(I18n::t("chat.action.denied"), false);
    });
    connect(onceBtn, &QPushButton::clicked, this, [this, op, snapshotJson, humanSummary, eventId, markResolved]{
        executeAction(op, snapshotJson, humanSummary, eventId);
        markResolved(I18n::t("chat.action.executed"), true);
    });
    connect(alwaysBtn, &QPushButton::clicked, this, [this, op, snapshotJson, humanSummary, eventId, markResolved]{
        // 持久化"总是允许"
        if (op == "add")    Preferences::instance().setAutoApproveAdd(true);
        if (op == "delete") Preferences::instance().setAutoApproveDelete(true);
        if (op == "update") Preferences::instance().setAutoApproveUpdate(true);
        executeAction(op, snapshotJson, humanSummary, eventId);
        markResolved(I18n::t("chat.action.executed"), true);
    });

    QTimer::singleShot(0, this, &ChatPage::scrollToBottom);
}

void ChatPage::executeAction(const QString &op, const QString &snapshotJson,
                             const QString &humanSummary, const QString &eventId) {
    CalendarEvent ev = eventFromJson(snapshotJson);
    if (op == "add") {
        m_db->insertEvent(ev);
    } else if (op == "delete") {
        m_db->deleteEvent(eventId);
    } else if (op == "update") {
        m_db->updateEvent(ev);
    }
    m_db->recordChatAction(op, eventId, snapshotJson, humanSummary);
    if (m_drawerOpen) reloadActionDrawer();
}

void ChatPage::undoAction(const ChatAction &a) {
    // 撤销逻辑：add → delete；delete → re-insert snapshot；update → restore snapshot
    if (a.op == "add") {
        m_db->deleteEvent(a.eventId);
    } else if (a.op == "delete") {
        CalendarEvent ev = eventFromJson(a.snapshotJson);
        if (!ev.id.isEmpty()) m_db->insertEvent(ev);
    } else if (a.op == "update") {
        CalendarEvent ev = eventFromJson(a.snapshotJson);
        if (!ev.id.isEmpty()) m_db->updateEvent(ev);
    }
    m_db->deleteChatActionRecord(a.id);
    if (m_drawerOpen) reloadActionDrawer();
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

        QFrame#ChatActionDrawer {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }
        QLabel#ChatActionColHeaderAdd,
        QLabel#ChatActionColHeaderDel {
            font-size: 13px;
            font-weight: 700;
            color: %3;
            padding: 2px 0 6px 0;
            border-bottom: 1px solid %2;
        }
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
