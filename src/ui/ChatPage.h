#pragma once

#include <QWidget>
#include <QVector>

class QScrollArea;
class QVBoxLayout;
class QLineEdit;
class QPushButton;
class QLabel;

namespace timeplan {

class DeepSeekClient;
class Database;

/**
 * AI 对话页面
 * - 用户输入框 + 发送按钮 (Cmd/Ctrl+Enter 发送)
 * - 消息列表（左侧 AI 灰底，右侧用户红底）
 * - 流式追加 AI 回复
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
    QString systemPrompt() const;

    Database *m_db;
    DeepSeekClient *m_ai;

    QScrollArea *m_scroll;
    QWidget *m_msgContainer;
    QVBoxLayout *m_msgLayout;
    QLineEdit *m_input;
    QPushButton *m_sendBtn;
    QPushButton *m_clearBtn;
    QLabel *m_emptyHint;

    QLabel *m_currentStreamingBubble = nullptr;
    QString m_streamingText;
    bool m_isResponding = false;
};

} // namespace timeplan
