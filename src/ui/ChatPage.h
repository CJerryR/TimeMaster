#pragma once

#include <QWidget>
#include <QVector>

class QScrollArea;
class QVBoxLayout;
class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;

namespace timemaster {

class DeepSeekClient;
class Database;

/**
 * AI 对话页面
 *  - 顶部：上下文开关（是否把日历传给 AI）
 *  - 中部：消息流（用户右红色气泡 + AI 左灰色气泡）
 *  - 底部：输入框 + 发送按钮（Enter 发送）
 *
 * 关键改进：每次发送前，自动从 DB 抓取 ±N 天的日历事件，
 * 格式化为紧凑文本注入到 AI 的 system context 中，
 * AI 因此能回答「下周三我有什么安排」「明天有空吗」之类的问题。
 */
class ChatPage : public QWidget {
    Q_OBJECT
public:
    explicit ChatPage(Database *db, DeepSeekClient *ai, QWidget *parent = nullptr);

private slots:
    void applyTheme();
    void onSend();
    void onChatChunk(const QString &delta);
    void onChatFinished(const QString &full);
    void onChatError(const QString &msg);
    void onClear();

private:
    QLabel *appendBubble(const QString &text, bool isUser, bool isStreaming = false);
    void scrollToBottom();

    QString basePrompt() const;
    QString buildCalendarContext() const;

    Database *m_db;
    DeepSeekClient *m_ai;

    QScrollArea *m_scroll;
    QWidget *m_msgContainer;
    QVBoxLayout *m_msgLayout;
    QLineEdit *m_input;
    QPushButton *m_sendBtn;
    QPushButton *m_clearBtn;
    QCheckBox *m_useCtxCheck;
    QLabel *m_ctxStatus;
    QLabel *m_emptyHint;

    QLabel *m_currentStreamingBubble = nullptr;
    QString m_streamingText;
    bool m_isResponding = false;
};

} // namespace timemaster
