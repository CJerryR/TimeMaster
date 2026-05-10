#pragma once

#include "Types.h"
#include <QString>
#include <QSqlDatabase>
#include <QObject>

namespace timeplan {

/**
 * SQLite 数据库管理
 * 数据库文件存储在应用所在文件夹:
 *   与可执行文件同级目录（.app 包外）
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
    bool insertEvent(const CalendarEvent &event);
    bool updateEvent(const CalendarEvent &event);
    bool deleteEvent(const QString &id);

    // ---- 统计 ----
    QList<CategoryStat> getCategoryStats(const QDateTime &start, const QDateTime &end);
    QList<DailySummary> getDailySummaries(const QDateTime &start, const QDateTime &end);
    QList<HourlyBucket> getHourlyDistribution(const QDateTime &start, const QDateTime &end);
    int eventCountBySource(EventSource source, const QDateTime &start, const QDateTime &end);

    // 工具：生成 UUID
    static QString generateId();

signals:
    void eventsChanged();

private:
    bool initSchema();
    CalendarEvent rowToEvent(const QSqlQuery &q);

    QSqlDatabase m_db;
    QString m_dbPath;
    QString m_connectionName;
};

} // namespace timeplan
