//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QWidget>
#include <QVector>

class QScrollArea;
class QVBoxLayout;
class QLineEdit;
class QPushButton;
class QLabel;
class QFrame;
class QToolButton;

namespace timemaster {

class DeepSeekClient;
class Database;

/**
 * V4.3 chat page.
 *  · Top bar: privacy chip (settings shortcut), action history toggle.
 *  · Empty state with suggestion bubbles.
 *  · NEW V4.3 #7: AI 可以通过返回 ```action JSON 块来请求操作日历（增/删/改），
 *    UI 把每个 action 渲染成一张审批卡（允许 / 拒绝 / 总是允许），
 *    用户同意后才真正写库。所有审批通过的操作都进入 chat_actions 表，
 *    可在顶部"操作历史"抽屉里查看 + 撤销。
 */
class ChatPage : public QWidget {
    Q_OBJECT
public:
    explicit ChatPage(Database *db, DeepSeekClient *ai, QWidget *parent = nullptr);

signals:
    void openSettingsRequested();

protected:
    void resizeEvent(QResizeEvent *e) override;

public slots:
    void refreshFromSettings();

private slots:
    void applyTheme();
    void applyLanguage();
    void onSend();
    void onChatChunk(const QString &delta);
    void onChatFinished(const QString &full);
    void onChatError(const QString &message);
    void onClear();
    void onPrivacyChipClicked();
    void onHistoryDrawerToggled();
    void reloadActionDrawer();

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

    // ---- V4.3 #7 action parsing + approval ----
    // 解析 AI 完整回复，提取 ```action ...``` JSON 块，每个块渲染一张审批卡。
    void parseAndRenderActions(const QString &fullAiText);
    void appendActionCard(const QString &op, const QString &humanSummary,
                          const QString &snapshotJson, const QString &eventId);
    void executeAction(const QString &op, const QString &snapshotJson,
                       const QString &humanSummary, const QString &eventId);
    void undoAction(const ChatAction &a);

    Database *m_db;
    DeepSeekClient *m_ai;

    QScrollArea  *m_scroll       = nullptr;
    QWidget      *m_msgContainer = nullptr;
    QVBoxLayout  *m_msgLayout    = nullptr;
    QLineEdit    *m_input        = nullptr;
    QPushButton  *m_sendBtn      = nullptr;
    QPushButton  *m_clearBtn     = nullptr;
    QPushButton  *m_privacyChip  = nullptr;
    QPushButton  *m_historyBtn   = nullptr;  // V4.3 #7 — 顶栏的"操作历史"切换按钮
    QLabel       *m_titleLabel   = nullptr;
    QLabel       *m_titleIcon    = nullptr;

    // V4.3 #7 — 操作历史抽屉（可折叠）
    QFrame       *m_historyDrawer = nullptr;
    QVBoxLayout  *m_drawerAddedColumn   = nullptr;
    QVBoxLayout  *m_drawerDeletedColumn = nullptr;
    QLabel       *m_drawerAddedHeader   = nullptr;
    QLabel       *m_drawerDeletedHeader = nullptr;
    bool          m_drawerOpen = false;

    QWidget      *m_emptyState   = nullptr;
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
