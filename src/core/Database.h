//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "Types.h"
#include <QString>
#include <QSqlDatabase>
#include <QObject>
#include <optional>

namespace timemaster {

// SQLite 数据库管理器：管理 events、ai_batches、chat_messages、chat_actions 四张表
/**
 * SQLite 数据库管理
 *  - 事件 CRUD + 统计
 *  - AI 导入批次（用于撤销 AI 解析结果）
 *    - 每次 AI 解析记一条 batch，事件通过 ai_batch_id 关联
 *    - 可以删除整个 batch（及其所有事件），或单条事件
 */
class Database : public QObject {
    Q_OBJECT
public:
    // 构造函数：初始化数据库连接名
    explicit Database(QObject *parent = nullptr);
    // 析构函数：关闭并移除数据库连接
    ~Database();

    // 打开/创建 SQLite 数据库，启用 WAL 模式和外键，自动建表迁移
    bool open();
    // 返回数据库文件的完整路径
    QString filePath() const { return m_dbPath; }

    // ---- 事件 CRUD ----
    // 获取全部事件，按起始时间升序排列
    QList<CalendarEvent> getAllEvents();
    // 按时间范围重叠查询事件列表
    QList<CalendarEvent> getEventsByRange(const QDateTime &start, const QDateTime &end);
    // 按 ID 查询单条事件，用于 ChatPage 审批撤销时的快照
    // V4.3 #7 — ChatPage 用：撤销删除/修改时需要先取原事件做 snapshot
    std::optional<CalendarEvent> getEventById(const QString &id);
    // 插入新事件，发射 eventsChanged 信号
    bool insertEvent(const CalendarEvent &event);
    // 更新事件可变字段，发射 eventsChanged 信号
    bool updateEvent(const CalendarEvent &event);
    // 按 ID 删除事件，发射 eventsChanged 信号
    bool deleteEvent(const QString &id);

    // ---- AI 批次 ----
    // 创建 AI 导入批次记录，返回批次 UUID
    // 创建一个 batch，返回 batch_id
    QString createAiBatch(const QString &sourceText, const QString &sourceType = "parse");
    // 获取所有 AI 批次，附带存活事件计数
    // 获取所有批次（按创建时间倒序）
    QList<AiBatchInfo> getAiBatches();
    // 获取某批次下的所有事件
    // 获取某批次内的所有事件（包含已删除？默认仅存活的）
    QList<CalendarEvent> getBatchEvents(const QString &batchId);
    // 删除整批事件和批次记录
    // 撤销整个批次：删除批次记录 + 删除其下所有事件
    bool deleteBatch(const QString &batchId);
    // 只删除批次记录，保留事件
    // 仅删除批次记录本身（不删除事件，用于"保留事件、清理历史"）
    bool clearBatchRecord(const QString &batchId);

    // ---- 统计 ----
    // 统计时间范围内各类别的总时长、事件数和百分比
    QList<CategoryStat> getCategoryStats(const QDateTime &start, const QDateTime &end);
    // 按天聚合每日总时长和事件数
    QList<DailySummary> getDailySummaries(const QDateTime &start, const QDateTime &end);
    // 统计 0-23 各小时段的总时长分布
    QList<HourlyBucket> getHourlyDistribution(const QDateTime &start, const QDateTime &end);
    // 按来源类型统计事件数量
    int eventCountBySource(EventSource source, const QDateTime &start, const QDateTime &end);

    // ---- V4.3 #7 AI 对话动作历史 ----
    // 每次对话页面里 AI 通过审批卡执行的操作（add / delete / update）都记一条，
    // 用于动作历史抽屉和"撤销最近操作"功能。
    // 记录一条 AI 聊天操作日志（增/删/改）
    bool recordChatAction(const QString &op, const QString &eventId,
                          const QString &snapshotJson, const QString &humanSummary);
    // 获取最近 N 条操作记录，用于历史抽屉
    QList<ChatAction> getRecentChatActions(int limit = 50);
    // 删除单条操作记录
    bool deleteChatActionRecord(const QString &id);

    // 生成 UUID 格式的唯一 ID
    static QString generateId();

signals:
    // 事件增删改时发射，驱动 UI 刷新
    void eventsChanged();
    // AI 批次变更时发射
    void aiBatchesChanged();

private:
    // 创建四张核心表及索引
    bool initSchema();
    // 为旧版本数据库补充新增列
    void migrateSchema();
    // 将 SQL 查询结果行转为 CalendarEvent 结构体
    CalendarEvent rowToEvent(const QSqlQuery &q);

    QSqlDatabase m_db;
    QString m_dbPath;
    QString m_connectionName;
};

} // namespace timemaster
