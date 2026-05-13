//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QWidget>
#include <QVector>
#include <QJsonArray>

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
// AI 对话页：多轮记忆聊天、日历上下文注入、AI 操作审批卡、操作历史抽屉
class ChatPage : public QWidget {
    Q_OBJECT
public:
    // 构造函数
    explicit ChatPage(Database *db, DeepSeekClient *ai, QWidget *parent = nullptr);

signals:
    // 打开设置请求
    void openSettingsRequested();

protected:
    // 窗口大小变化事件
    void resizeEvent(QResizeEvent *e) override;

public slots:
    // 从设置刷新隐私芯片状态
    void refreshFromSettings();

private slots:
    // 应用主题
    void applyTheme();
    // 应用语言
    void applyLanguage();
    // 发送消息
    void onSend();
    // 流式接收 AI 回复片段
    void onChatChunk(const QString &delta);
    // AI 回复完成
    void onChatFinished(const QString &full);
    // AI 回复出错
    void onChatError(const QString &message);
    // 清空对话
    void onClear();
    // 隐私芯片点击 → 打开设置
    void onPrivacyChipClicked();
    // 操作历史抽屉切换
    void onHistoryDrawerToggled();
    // 重新加载操作历史抽屉
    void reloadActionDrawer();

private:
    // 添加聊天气泡
    QLabel *appendBubble(const QString &text, bool isUser, bool isStreaming = false);
    // 滚动到底部
    void scrollToBottom();
    // 更新气泡最大宽度
    void updateBubblesMaxWidth();
    // 重建空状态提示
    void rebuildEmptyState();
    // 发送预制快捷查询
    void sendCannedQuery(const QString &q);

    // 构造 AI 系统提示词
    QString basePrompt() const;
    // 构造日历上下文文本
    QString buildCalendarContext() const;
    // 更新隐私芯片文本和样式
    void    updatePrivacyChip();

    // AI 是否可见日历
    bool aiSeesCalendar() const { return m_aiSeesCalendar; }
    // 设置 AI 日历可见性
    void setAiSeesCalendar(bool v);

    // ---- V4.3 #7 action parsing + approval ----
    // 解析 AI 完整回复，提取 ```action ...``` JSON 块，每个块渲染一张审批卡。
    // 解析 AI 回复中的 action 块并渲染审批卡
    void parseAndRenderActions(const QString &fullAiText);
    // 追加操作审批卡到对话
    void appendActionCard(const QString &op, const QString &humanSummary,
                          const QString &snapshotJson, const QString &eventId);
    // 执行日历操作（增/删/改）
    void executeAction(const QString &op, const QString &snapshotJson,
                       const QString &humanSummary, const QString &eventId);
    // 撤销已执行的操作
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

    // V4.3.3 #1 — 多轮对话记忆。每次 onSend 都把这个 history（user / assistant
    // 交替）拼到请求里发出去，AI 才会记得师傅上一句聊了什么。当前一轮的
    // user 消息会缓存在 m_pendingUserMessage，只有 chatFinished 时才正式
    // 合并到 m_chatHistory；如果中途报错就丢掉，避免脏数据污染下一轮。
    QJsonArray m_chatHistory;
    QString    m_pendingUserMessage;
};

} // namespace timemaster
