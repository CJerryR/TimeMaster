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
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>
#include <QDateTime>

namespace timeplan {

ChatPage::ChatPage(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QWidget(parent), m_db(db), m_ai(ai)
{
    setObjectName("ChatPage");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---- 顶部栏 ----
    auto *topBar = new QWidget;
    topBar->setObjectName("ChatTopBar");
    topBar->setFixedHeight(56);
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);

    auto *title = new QLabel("AI 对话");
    title->setObjectName("PageTitle");

    m_clearBtn = new QPushButton("清空");
    m_clearBtn->setObjectName("SecondaryBtn");
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, &ChatPage::onClear);

    topLayout->addWidget(title);
    topLayout->addStretch();
    topLayout->addWidget(m_clearBtn);
    root->addWidget(topBar);

    // ---- 消息滚动区 ----
    m_scroll = new QScrollArea;
    m_scroll->setObjectName("ChatScroll");
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_msgContainer = new QWidget;
    m_msgContainer->setObjectName("ChatMsgContainer");
    m_msgLayout = new QVBoxLayout(m_msgContainer);
    m_msgLayout->setContentsMargins(40, 24, 40, 24);
    m_msgLayout->setSpacing(14);

    m_emptyHint = new QLabel("👋 你好，我是时智 AI 助手\n\n你可以问我：\n· 帮我安排明天的工作计划\n· 我下周三有什么安排\n· 时间管理建议");
    m_emptyHint->setObjectName("ChatEmptyHint");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_emptyHint->setWordWrap(true);
    m_msgLayout->addWidget(m_emptyHint, 0, Qt::AlignCenter);
    m_msgLayout->addStretch();

    m_scroll->setWidget(m_msgContainer);
    root->addWidget(m_scroll, 1);

    // ---- 输入区 ----
    auto *inputBar = new QWidget;
    inputBar->setObjectName("ChatInputBar");
    auto *inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(40, 12, 40, 16);
    inputLayout->setSpacing(8);

    m_input = new QLineEdit;
    m_input->setObjectName("ChatInput");
    m_input->setPlaceholderText("输入消息，按 Enter 发送...");
    m_input->setMinimumHeight(40);

    m_sendBtn = new QPushButton("发送");
    m_sendBtn->setObjectName("PrimaryBtn");
    m_sendBtn->setMinimumHeight(40);
    m_sendBtn->setMinimumWidth(80);
    m_sendBtn->setCursor(Qt::PointingHandCursor);

    connect(m_input, &QLineEdit::returnPressed, this, &ChatPage::onSend);
    connect(m_sendBtn, &QPushButton::clicked, this, &ChatPage::onSend);

    inputLayout->addWidget(m_input);
    inputLayout->addWidget(m_sendBtn);
    root->addWidget(inputBar);

    // ---- 信号连接 ----
    connect(m_ai, &DeepSeekClient::chatChunk, this, &ChatPage::onChatChunk);
    connect(m_ai, &DeepSeekClient::chatFinished, this, &ChatPage::onChatFinished);
    connect(m_ai, &DeepSeekClient::chatError, this, &ChatPage::onChatError);

    connect(&Theme::instance(), &Theme::changed, this, &ChatPage::applyTheme);
    applyTheme();
}

QString ChatPage::systemPrompt() const {
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd dddd");
    return QString(
        "你是时智 AI，一个专业且友善的智能日程管理助手。\n"
        "今天是 %1。\n"
        "请用简洁、有条理的中文回答用户问题。\n"
        "如果用户在描述具体的日程安排，请引导他们使用日历页顶部的 "
        "「AI 解析」 输入框，那里可以一键创建。"
    ).arg(today);
}

void ChatPage::onSend() {
    QString text = m_input->text().trimmed();
    if (text.isEmpty() || m_isResponding) return;

    if (!m_ai->hasApiKey()) {
        appendBubble("请先在 ⚙ 设置 中配置 DeepSeek API Key。", false);
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

    m_ai->sendChat(text, systemPrompt());
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
    // 移除 stretch 之前的所有消息气泡
    while (m_msgLayout->count() > 0) {
        auto *item = m_msgLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_emptyHint = new QLabel("👋 你好，我是时智 AI 助手\n\n你可以问我：\n· 帮我安排明天的工作计划\n· 我下周三有什么安排\n· 时间管理建议");
    m_emptyHint->setObjectName("ChatEmptyHint");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_emptyHint->setWordWrap(true);
    m_msgLayout->addWidget(m_emptyHint, 0, Qt::AlignCenter);
    m_msgLayout->addStretch();
    m_currentStreamingBubble = nullptr;
    m_isResponding = false;
    m_sendBtn->setEnabled(true);
    applyTheme();
}

QLabel *ChatPage::appendBubble(const QString &text, bool isUser, bool isStreaming) {
    Q_UNUSED(isStreaming);

    // 移除 stretch
    int stretchIdx = -1;
    for (int i = m_msgLayout->count() - 1; i >= 0; --i) {
        if (m_msgLayout->itemAt(i)->spacerItem()) {
            stretchIdx = i;
            break;
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
    bubble->setMaximumWidth(640);
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
    Theme &t = Theme::instance();
    QString brand = t.brand().name();
    QString brandHover = t.brand().darker(110).name();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();
    QString stroke = t.stroke().name();
    QString bgPage = t.bgPage().name();
    QString bgContainer = t.bgContainer().name();
    QString bgComp = t.bgComponent().name();
    QString bgHover = t.bgHover().name();

    setStyleSheet(QString(R"(
        QWidget#ChatPage { background-color: %1; }
        QWidget#ChatTopBar {
            background-color: %2;
            border-bottom: 1px solid %3;
        }
        QLabel#PageTitle {
            font-size: 18px;
            font-weight: 600;
            color: %4;
        }
        QScrollArea#ChatScroll { background-color: %1; border: none; }
        QWidget#ChatMsgContainer { background-color: %1; }
        QLabel#ChatEmptyHint {
            color: %5;
            font-size: 14px;
            line-height: 1.6;
            padding: 60px 0;
        }
        QLabel#ChatBubbleUser {
            background-color: %6;
            color: white;
            border-radius: 14px;
            padding: 10px 14px;
            font-size: 14px;
        }
        QLabel#ChatBubbleAI {
            background-color: %7;
            color: %4;
            border-radius: 14px;
            padding: 10px 14px;
            font-size: 14px;
            border: 1px solid %3;
        }
        QWidget#ChatInputBar {
            background-color: %2;
            border-top: 1px solid %3;
        }
        QLineEdit#ChatInput {
            background-color: %8;
            color: %4;
            border: 1px solid %3;
            border-radius: 10px;
            padding: 0 12px;
            font-size: 14px;
        }
        QLineEdit#ChatInput:focus { border-color: %6; }
        QPushButton#PrimaryBtn {
            background-color: %6;
            color: white;
            border: none;
            border-radius: 10px;
            font-weight: 500;
            padding: 0 14px;
        }
        QPushButton#PrimaryBtn:hover { background-color: %9; }
        QPushButton#PrimaryBtn:disabled { background-color: %3; color: %5; }
        QPushButton#SecondaryBtn {
            background-color: transparent;
            color: %5;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 4px 12px;
        }
        QPushButton#SecondaryBtn:hover { background-color: %8; color: %4; }
    )")
        .arg(bgPage, bgContainer, stroke, textPrim, textSec, brand, bgComp, bgHover, brandHover));
}

} // namespace timeplan
