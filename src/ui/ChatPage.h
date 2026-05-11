#pragma once

#include <QWidget>
#include <QVector>

class QScrollArea;
class QVBoxLayout;
class QLineEdit;
class QPushButton;
class QLabel;

namespace timemaster {

class DeepSeekClient;
class Database;

/**
 * V4 chat page.
 *  · Top bar: privacy chip (replaces the orphan checkbox) summarising the
 *    AI-sees-calendar state. Click → opens Settings.
 *  · Empty state: clean professional copy with three clickable suggestion
 *    bubbles (no "~呀啦哦人家" particles, no self-anthropomorphic AI persona).
 *  · Persona is now a concise, professional assistant.
 */
class ChatPage : public QWidget {
    Q_OBJECT
public:
    explicit ChatPage(Database *db, DeepSeekClient *ai, QWidget *parent = nullptr);

signals:
    void openSettingsRequested();

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void applyTheme();
    void applyLanguage();
    void onSend();
    void onChatChunk(const QString &delta);
    void onChatFinished(const QString &full);
    void onChatError(const QString &msg);
    void onClear();
    void onPrivacyChipClicked();

private:
    QLabel *appendBubble(const QString &text, bool isUser, bool isStreaming = false);
    void scrollToBottom();
    void updateBubblesMaxWidth();
    void rebuildEmptyState();
    void sendCannedQuery(const QString &q);

    QString basePrompt() const;
    QString buildCalendarContext() const;
    void    updatePrivacyChip();

    bool aiSeesCalendar() const { return m_aiSeesCalendar; }
    void setAiSeesCalendar(bool v);

    Database *m_db;
    DeepSeekClient *m_ai;

    QScrollArea  *m_scroll       = nullptr;
    QWidget      *m_msgContainer = nullptr;
    QVBoxLayout  *m_msgLayout    = nullptr;
    QLineEdit    *m_input        = nullptr;
    QPushButton  *m_sendBtn      = nullptr;
    QPushButton  *m_clearBtn     = nullptr;
    QPushButton  *m_privacyChip  = nullptr;
    QLabel       *m_titleLabel   = nullptr;
    QLabel       *m_titleIcon    = nullptr;

    QWidget      *m_emptyState   = nullptr;  // 整个空状态卡（含建议气泡）
    QLabel       *m_emptyTitle   = nullptr;
    QLabel       *m_emptySubtitle= nullptr;
    QVector<QPushButton*> m_suggestionButtons;

    QList<QLabel*> m_bubbles;

    QLabel  *m_currentStreamingBubble = nullptr;
    QString  m_streamingText;
    bool     m_isResponding   = false;
    bool     m_aiSeesCalendar = true;
};

} // namespace timemaster
