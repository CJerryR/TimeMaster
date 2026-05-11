#include "ChatPage.h"
#include "Theme.h"
#include "../core/DeepSeekClient.h"
#include "../core/Database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>
#include <QDateTime>
#include <QFrame>

namespace timemaster {

ChatPage::ChatPage(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QWidget(parent), m_db(db), m_ai(ai)
{
    setObjectName("ChatPage");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 18, 20, 18);
    root->setSpacing(12);

    // ---- 顶部条 ----
    auto *topBar = new QFrame;
    topBar->setObjectName("ChatTopBar");
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(18, 12, 18, 12);
    topLayout->setSpacing(12);

    auto *title = new QLabel("💬  AI 对话");
    title->setObjectName("ChatPageTitle");

    m_useCtxCheck = new QCheckBox("让 AI 看到我的日历");
    m_useCtxCheck->setObjectName("CtxCheck");
    m_useCtxCheck->setChecked(true);
    m_useCtxCheck->setToolTip("勾选后，每次提问会把过去 7 天与未来 14 天的日程自动发给 AI");
    m_useCtxCheck->setCursor(Qt::PointingHandCursor);

    m_ctxStatus = new QLabel;
    m_ctxStatus->setObjectName("CtxStatus");

    m_clearBtn = new QPushButton("清空对话");
    m_clearBtn->setObjectName("ChatGhostBtn");
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, &ChatPage::onClear);

    topLayout->addWidget(title);
    topLayout->addStretch();
    topLayout->addWidget(m_ctxStatus);
    topLayout->addWidget(m_useCtxCheck);
    topLayout->addWidget(m_clearBtn);
    root->addWidget(topBar);

    // ---- 消息滚动区（卡片样式） ----
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

    m_emptyHint = new QLabel(
        "👋 你好，我是时间管理大师 AI 助手\n\n"
        "你可以问我：\n"
        "· 我下周三有什么安排？\n"
        "· 帮我规划一下明天的工作\n"
        "· 这周哪天最忙？\n"
        "· 给我一些时间管理建议"
    );
    m_emptyHint->setObjectName("ChatEmptyHint");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_emptyHint->setWordWrap(true);
    m_msgLayout->addWidget(m_emptyHint, 0, Qt::AlignCenter);
    m_msgLayout->addStretch();

    m_scroll->setWidget(m_msgContainer);
    chatCardLayout->addWidget(m_scroll);
    root->addWidget(chatCard, 1);

    // ---- 输入区 ----
    auto *inputCard = new QFrame;
    inputCard->setObjectName("ChatInputCard");
    auto *inputLayout = new QHBoxLayout(inputCard);
    inputLayout->setContentsMargins(14, 10, 10, 10);
    inputLayout->setSpacing(10);

    m_input = new QLineEdit;
    m_input->setObjectName("ChatInput");
    m_input->setPlaceholderText("输入消息，按 Enter 发送…");
    m_input->setMinimumHeight(40);

    m_sendBtn = new QPushButton("发送 →");
    m_sendBtn->setObjectName("ChatSendBtn");
    m_sendBtn->setMinimumHeight(40);
    m_sendBtn->setMinimumWidth(90);
    m_sendBtn->setCursor(Qt::PointingHandCursor);

    connect(m_input, &QLineEdit::returnPressed, this, &ChatPage::onSend);
    connect(m_sendBtn, &QPushButton::clicked, this, &ChatPage::onSend);

    inputLayout->addWidget(m_input);
    inputLayout->addWidget(m_sendBtn);
    root->addWidget(inputCard);

    // ---- 信号连接 ----
    connect(m_ai, &DeepSeekClient::chatChunk, this, &ChatPage::onChatChunk);
    connect(m_ai, &DeepSeekClient::chatFinished, this, &ChatPage::onChatFinished);
    connect(m_ai, &DeepSeekClient::chatError, this, &ChatPage::onChatError);

    connect(&Theme::instance(), &Theme::changed, this, &ChatPage::applyTheme);
    applyTheme();
}

QString ChatPage::basePrompt() const {
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd dddd");
    return QString(
        "你是「时间管理大师」（Time Master），一位专业的智能时间规划顾问。\n"
        "今天是 %1。\n"
        "回复风格：简洁、结构化、可操作的中文，要点突出，不啰嗦。\n"
        "如果下方提供了「用户当前日历」，请实事求是地基于该日历回答；"
        "若用户询问的某天没有安排，明确告诉他「这天暂无安排」而不要凭空生成日程。\n"
        "当用户描述具体的待办时，可以建议他用日历页的「AI 解析」一键创建。"
    ).arg(today);
}

QString ChatPage::buildCalendarContext() const {
    if (!m_db) return QString();

    QDateTime start(QDate::currentDate().addDays(-7), QTime(0, 0, 0));
    QDateTime end(QDate::currentDate().addDays(14),   QTime(23, 59, 59));
    auto events = m_db->getEventsByRange(start, end);

    if (events.isEmpty()) {
        return QString("【用户当前日历】\n（过去 7 天 ~ 未来 14 天范围内暂无任何日程）");
    }

    QString lines;
    lines += "【用户当前日历】（过去 7 天 ~ 未来 14 天）\n";

    // 限制条数避免 token 过量
    const int kMaxEvents = 80;
    int count = qMin(int(events.size()), kMaxEvents);

    for (int i = 0; i < count; ++i) {
        const auto &e = events[i];
        QString timeStr;
        if (e.allDay) {
            timeStr = e.startDate.toString("yyyy-MM-dd") + " 全天";
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

        QString line = QString("· %1 | %2").arg(timeStr, e.title);
        line += QString(" (%1").arg(categoryLabel(e.category));
        if (e.priority == EventPriority::Urgent) line += " · 紧急";
        line += ")";
        if (!e.location.isEmpty()) line += QString(" @ %1").arg(e.location);
        lines += line + "\n";
    }

    if (events.size() > kMaxEvents) {
        lines += QString("（共 %1 条，仅展示前 %2 条）\n").arg(events.size()).arg(kMaxEvents);
    }
    return lines;
}

void ChatPage::onSend() {
    QString text = m_input->text().trimmed();
    if (text.isEmpty() || m_isResponding) return;

    if (!m_ai->hasApiKey()) {
        appendBubble("请先在 ⚙ 设置中配置 DeepSeek API Key。", false);
        return;
    }

    if (m_emptyHint) {
        m_emptyHint->hide();
    }

    appendBubble(text, true);
    m_input->clear();

    m_isResponding = true;
    m_sendBtn->setEnabled(false);
    m_streamingText.clear();
    m_currentStreamingBubble = appendBubble("…", false, true);

    QString ctx;
    if (m_useCtxCheck->isChecked()) {
        ctx = buildCalendarContext();
        // 顶部状态条提示
        m_ctxStatus->setText("📅 已附带日历");
    } else {
        m_ctxStatus->setText("");
    }

    m_ai->sendChat(text, basePrompt(), ctx);
}

void ChatPage::onChatChunk(const QString &delta) {
    if (!m_currentStreamingBubble) return;
    m_streamingText += delta;
    m_currentStreamingBubble->setText(m_streamingText);
    scrollToBottom();
}

void ChatPage::onChatFinished(const QString &full) {
    if (m_currentStreamingBubble) {
        m_currentStreamingBubble->setText(full.isEmpty() ? m_streamingText : full);
        m_currentStreamingBubble = nullptr;
    }
    m_isResponding = false;
    m_sendBtn->setEnabled(true);
    scrollToBottom();
}

void ChatPage::onChatError(const QString &msg) {
    if (m_currentStreamingBubble) {
        m_currentStreamingBubble->setText("⚠ 出错了：" + msg);
        m_currentStreamingBubble = nullptr;
    } else {
        appendBubble("⚠ 出错了：" + msg, false);
    }
    m_isResponding = false;
    m_sendBtn->setEnabled(true);
}

void ChatPage::onClear() {
    while (m_msgLayout->count() > 0) {
        auto *item = m_msgLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_emptyHint = new QLabel(
        "👋 你好，我是时间管理大师 AI 助手\n\n"
        "你可以问我：\n"
        "· 我下周三有什么安排？\n"
        "· 帮我规划一下明天的工作\n"
        "· 这周哪天最忙？\n"
        "· 给我一些时间管理建议"
    );
    m_emptyHint->setObjectName("ChatEmptyHint");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_emptyHint->setWordWrap(true);
    m_msgLayout->addWidget(m_emptyHint, 0, Qt::AlignCenter);
    m_msgLayout->addStretch();
    m_currentStreamingBubble = nullptr;
    m_isResponding = false;
    m_sendBtn->setEnabled(true);
    m_ctxStatus->setText("");
    applyTheme();
}

QLabel *ChatPage::appendBubble(const QString &text, bool isUser, bool isStreaming) {
    Q_UNUSED(isStreaming);

    int stretchIdx = -1;
    for (int i = m_msgLayout->count() - 1; i >= 0; --i) {
        if (m_msgLayout->itemAt(i)->spacerItem()) {
            stretchIdx = i; break;
        }
    }
    if (stretchIdx >= 0) {
        auto *item = m_msgLayout->takeAt(stretchIdx);
        delete item;
    }

    auto *row = new QWidget;
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(0);

    auto *bubble = new QLabel(text);
    bubble->setWordWrap(true);
    bubble->setMaximumWidth(720);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bubble->setObjectName(isUser ? "ChatBubbleUser" : "ChatBubbleAI");

    if (isUser) {
        rowLayout->addStretch();
        rowLayout->addWidget(bubble);
    } else {
        rowLayout->addWidget(bubble);
        rowLayout->addStretch();
    }

    m_msgLayout->addWidget(row);
    m_msgLayout->addStretch();

    QTimer::singleShot(0, this, &ChatPage::scrollToBottom);
    return bubble;
}

void ChatPage::scrollToBottom() {
    auto *bar = m_scroll->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void ChatPage::applyTheme() {
    auto &t = Theme::instance();
    QString brand = t.brand().name();
    QString brandHover = t.brand().darker(110).name();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();
    QString placeholder = t.textPlaceholder().name();
    QString strokeR = t.strokeRgba();
    QString cardBg = t.cardBgRgba();
    QString componentBg = t.componentBgRgba();
    QString hoverBg = t.cardBgHoverRgba();

    setStyleSheet(QString(R"(
        QWidget#ChatPage { background: transparent; }

        QFrame#ChatTopBar {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }
        QLabel#ChatPageTitle {
            font-size: 16px;
            font-weight: 700;
            color: %3;
        }
        QCheckBox#CtxCheck {
            color: %3;
            font-size: 13px;
        }
        QCheckBox#CtxCheck::indicator {
            width: 16px; height: 16px;
        }
        QLabel#CtxStatus {
            color: %4;
            font-size: 12px;
            padding: 4px 10px;
            background-color: rgba(79,70,229,0.10);
            border-radius: 8px;
        }
        QPushButton#ChatGhostBtn {
            background-color: transparent;
            color: %5;
            border: 1px solid %2;
            border-radius: 9px;
            padding: 6px 14px;
        }
        QPushButton#ChatGhostBtn:hover { background-color: %6; color: %3; }

        QFrame#ChatCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 14px;
        }
        QScrollArea#ChatScroll { background-color: transparent; border: none; }
        QWidget#ChatMsgContainer { background-color: transparent; }
        QLabel#ChatEmptyHint {
            color: %5;
            font-size: 14px;
            line-height: 1.7;
            padding: 60px 0;
            background: transparent;
        }
        QLabel#ChatBubbleUser {
            background-color: %7;
            color: white;
            border-radius: 15px;
            padding: 11px 15px;
            font-size: 14px;
        }
        QLabel#ChatBubbleAI {
            background-color: %4;
            color: %3;
            border: 1px solid %2;
            border-radius: 15px;
            padding: 11px 15px;
            font-size: 14px;
        }

        QFrame#ChatInputCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 14px;
        }
        QLineEdit#ChatInput {
            background-color: transparent;
            color: %3;
            border: none;
            border-radius: 10px;
            padding: 0 8px;
            font-size: 14px;
        }
        QLineEdit#ChatInput:focus { border: none; }
        QPushButton#ChatSendBtn {
            background-color: %7;
            color: white;
            border: none;
            border-radius: 10px;
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
}

} // namespace timemaster
