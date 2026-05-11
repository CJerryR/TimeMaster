#include "ChatPage.h"
#include "Theme.h"
#include "IconRenderer.h"
#include "../core/DeepSeekClient.h"
#include "../core/Database.h"
#include "../utils/MarkdownToHtml.h"

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

namespace {
// 把单条消息文本绑定到一个 QLabel；如果是 AI 消息，做 Markdown → HTML 渲染
void bindMessageText(QLabel *bubble, const QString &text, bool isUser) {
    if (isUser) {
        bubble->setTextFormat(Qt::PlainText);
        bubble->setText(text);
    } else {
        // 流式渲染时也会触发，每次重新转一次
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

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 18, 20, 18);
    root->setSpacing(12);

    // ---- 顶部条 ----
    auto *topBar = new QFrame;
    topBar->setObjectName("ChatTopBar");
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(18, 12, 18, 12);
    topLayout->setSpacing(12);

    // 唯一的标题图标 —— 不要再创建第二份
    m_titleIcon = new QLabel;
    m_titleIcon->setObjectName("ChatPageTitleIcon");
    m_titleIcon->setFixedSize(22, 22);
    m_titleIcon->setPixmap(IconRenderer::pixmap(IconRenderer::NavChat, Theme::instance().brand(), 22));

    auto *title = new QLabel("AI 对话");
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

    topLayout->addWidget(m_titleIcon);
    topLayout->addWidget(title);
    topLayout->addStretch();
    topLayout->addWidget(m_ctxStatus);
    topLayout->addWidget(m_useCtxCheck);
    topLayout->addWidget(m_clearBtn);
    root->addWidget(topBar);

    // ---- 消息滚动区 ----
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
        "你好呀～我是你的专属时间秘书 ✿\n\n"
        "可以这样问我：\n"
        "· 我下周三都有什么安排呀？\n"
        "· 帮人家规划一下明天的工作好不好～\n"
        "· 这周哪天最忙呢？\n"
        "· 给我一些时间管理建议吧"
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
    m_input->setPlaceholderText("跟秘书说点什么吧，按 Enter 发送…");
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

    // ---- 信号 ----
    connect(m_ai, &DeepSeekClient::chatChunk, this, &ChatPage::onChatChunk);
    connect(m_ai, &DeepSeekClient::chatFinished, this, &ChatPage::onChatFinished);
    connect(m_ai, &DeepSeekClient::chatError, this, &ChatPage::onChatError);

    connect(&Theme::instance(), &Theme::changed, this, &ChatPage::applyTheme);
    applyTheme();
}

QString ChatPage::basePrompt() const {
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd dddd");
    // 「温柔可爱听话的女秘书」人设
    return QString(
        "你叫「小时」，是用户专属的私人时间秘书。\n"
        "性格设定：温柔、体贴、可爱、听话；以「主人」或「你」称呼用户，自己自称「小时」或「人家」。\n"
        "回复要求：\n"
        "1. 语气温柔亲切，时常带一点撒娇的语气词（如「啦」「呢」「呀」），但保持专业不腻歪。\n"
        "2. 内容专业、准确、可执行 —— 温柔是表面，靠谱是内核。\n"
        "3. 使用 Markdown 排版：**重点加粗**、列表分点、必要时用 `代码块` 引用具体时间或事件名。\n"
        "4. 篇幅克制：日常问题 80~200 字以内；规划类问题可以适度展开。\n"
        "5. 涉及日历的问题，严格基于下方「用户当前日历」实事求是地回答；如果某天没有安排，明确告诉主人「这天暂时是空的」，不要凭空编造。\n"
        "6. 鼓励主人用日历页顶部的「AI 解析」一键录入待办。\n\n"
        "今天是 %1。请用以上语气与主人对话。"
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
        appendBubble("主人～需要先去「⚙ 设置」里配置一下 DeepSeek API Key 哦，小时才能帮你做事呢。", false);
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
        m_ctxStatus->setText("📅 已附带日历");
    } else {
        m_ctxStatus->setText("");
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
    if (m_currentStreamingBubble) {
        bindMessageText(m_currentStreamingBubble,
                        "主人，刚刚出了一点小问题：" + msg, false);
        m_currentStreamingBubble = nullptr;
    } else {
        appendBubble("主人，刚刚出了一点小问题：" + msg, false);
    }
    m_isResponding = false;
    m_sendBtn->setEnabled(true);
}

void ChatPage::onClear() {
    m_bubbles.clear();
    while (m_msgLayout->count() > 0) {
        auto *item = m_msgLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_emptyHint = new QLabel(
        "你好呀～我是你的专属时间秘书 ✿\n\n"
        "可以这样问我：\n"
        "· 我下周三都有什么安排呀？\n"
        "· 帮人家规划一下明天的工作好不好～\n"
        "· 这周哪天最忙呢？\n"
        "· 给我一些时间管理建议吧"
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

    auto *bubble = new QLabel;
    bubble->setWordWrap(true);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bubble->setObjectName(isUser ? "ChatBubbleUser" : "ChatBubbleAI");
    bindMessageText(bubble, text, isUser);

    // 跟踪并按当前视口宽度设置 maxWidth
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
    // 减去容器左右内边距 (38*2 = 76)，再保留一点和滚动条 / 对侧 stretch 的余量
    int maxW = qMax(240, vw - 76 - 8);
    // 在大窗口上也不要让单个气泡占满整行，留一点呼吸空间
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
    QString placeholder = t.textPlaceholder().name();
    QString strokeR = t.strokeRgba();
    QString cardBg = t.cardBgRgba();
    QString componentBg = t.componentBgRgba();
    QString hoverBg = t.cardBgHoverRgba();

    if (m_titleIcon) {
        m_titleIcon->setPixmap(IconRenderer::pixmap(IconRenderer::NavChat, t.brand(), 22));
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
            color: %7;
            font-size: 12px;
            font-weight: 600;
            padding: 4px 12px;
            background-color: rgba(217,119,87,0.16);
            border: 1px solid rgba(217,119,87,0.32);
            border-radius: 10px;
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
            padding: 11px 16px;
            font-size: 14px;
        }
        QLabel#ChatBubbleAI {
            background-color: %4;
            color: %3;
            border: 1px solid %2;
            border-radius: 15px;
            padding: 12px 16px;
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

    if (m_titleIcon) {
        m_titleIcon->setPixmap(IconRenderer::pixmap(IconRenderer::NavChat, t.brand(), 22));
    }
}

} // namespace timemaster
