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
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool open();
    QString filePath() const { return m_dbPath; }

    // ---- 事件 CRUD ----
    QList<CalendarEvent> getAllEvents();
    QList<CalendarEvent> getEventsByRange(const QDateTime &start, const QDateTime &end);
    // V4.3 #7 — ChatPage 用：撤销删除/修改时需要先取原事件做 snapshot
    std::optional<CalendarEvent> getEventById(const QString &id);
    bool insertEvent(const CalendarEvent &event);
    bool updateEvent(const CalendarEvent &event);
    bool deleteEvent(const QString &id);

    // ---- AI 批次 ----
    // 创建一个 batch，返回 batch_id
    QString createAiBatch(const QString &sourceText, const QString &sourceType = "parse");
    // 获取所有批次（按创建时间倒序）
    QList<AiBatchInfo> getAiBatches();
    // 获取某批次内的所有事件（包含已删除？默认仅存活的）
    QList<CalendarEvent> getBatchEvents(const QString &batchId);
    // 撤销整个批次：删除批次记录 + 删除其下所有事件
    bool deleteBatch(const QString &batchId);
    // 仅删除批次记录本身（不删除事件，用于"保留事件、清理历史"）
    bool clearBatchRecord(const QString &batchId);

    // ---- 统计 ----
    QList<CategoryStat> getCategoryStats(const QDateTime &start, const QDateTime &end);
    QList<DailySummary> getDailySummaries(const QDateTime &start, const QDateTime &end);
    QList<HourlyBucket> getHourlyDistribution(const QDateTime &start, const QDateTime &end);
    int eventCountBySource(EventSource source, const QDateTime &start, const QDateTime &end);

    // ---- V4.3 #7 AI 对话动作历史 ----
    // 每次对话页面里 AI 通过审批卡执行的操作（add / delete / update）都记一条，
    // 用于动作历史抽屉和"撤销最近操作"功能。
    bool recordChatAction(const QString &op, const QString &eventId,
                          const QString &snapshotJson, const QString &humanSummary);
    QList<ChatAction> getRecentChatActions(int limit = 50);
    bool deleteChatActionRecord(const QString &id);

    static QString generateId();

signals:
    void eventsChanged();
    void aiBatchesChanged();

private:
    bool initSchema();
    void migrateSchema();
    CalendarEvent rowToEvent(const QSqlQuery &q);

    QSqlDatabase m_db;
    QString m_dbPath;
    QString m_connectionName;
};

} // namespace timemaster
