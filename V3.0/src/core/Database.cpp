#include "Database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QDir>
#include <QUuid>
#include <QDebug>
#include <QVariant>
namespace timemaster {

Database::Database(QObject *parent) : QObject(parent) {
    m_connectionName = QStringLiteral("timemaster-main");
}

Database::~Database() {
    if (m_db.isOpen()) m_db.close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::open() {
    QString dataDir = QCoreApplication::applicationDirPath();
    QDir().mkpath(dataDir);
    m_dbPath = dataDir + "/timemaster.db";

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode = WAL");
    q.exec("PRAGMA foreign_keys = ON");

    if (!initSchema()) return false;
    migrateSchema();
    return true;
}

bool Database::initSchema() {
    QSqlQuery q(m_db);

    bool ok = q.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS events (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT DEFAULT '',
            start_date TEXT NOT NULL,
            end_date TEXT NOT NULL,
            all_day INTEGER DEFAULT 0,
            color TEXT DEFAULT 'blue',
            category TEXT DEFAULT 'other',
            location TEXT DEFAULT '',
            reminder INTEGER DEFAULT 15,
            priority TEXT DEFAULT 'normal',
            source TEXT DEFAULT 'manual',
            ai_batch_id TEXT DEFAULT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        )
    )SQL");
    if (!ok) {
        qWarning() << "Create events table failed:" << q.lastError().text();
        return false;
    }

    q.exec("CREATE INDEX IF NOT EXISTS idx_events_start ON events(start_date)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_events_category ON events(category)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_events_batch ON events(ai_batch_id)");

    // AI 批次表
    ok = q.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS ai_batches (
            id TEXT PRIMARY KEY,
            source_text TEXT NOT NULL,
            source_type TEXT DEFAULT 'parse',
            event_count INTEGER DEFAULT 0,
            created_at TEXT NOT NULL
        )
    )SQL");
    if (!ok) {
        qWarning() << "Create ai_batches table failed:" << q.lastError().text();
    }

    q.exec("CREATE INDEX IF NOT EXISTS idx_batches_created ON ai_batches(created_at)");

    // 简单的对话历史
    q.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS chat_messages (
            id TEXT PRIMARY KEY,
            role TEXT NOT NULL,
            content TEXT NOT NULL,
            created_at TEXT NOT NULL
        )
    )SQL");

    return true;
}

void Database::migrateSchema() {
    // 旧版数据库可能没有 ai_batch_id 列，尝试添加（失败说明已存在）
    QSqlQuery q(m_db);
    q.exec("ALTER TABLE events ADD COLUMN ai_batch_id TEXT DEFAULT NULL");
    // 注意：q.lastError() 在列已存在时会报错，正常忽略
}

QString Database::generateId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

CalendarEvent Database::rowToEvent(const QSqlQuery &q) {
    CalendarEvent e;
    e.id = q.value("id").toString();
    e.title = q.value("title").toString();
    e.description = q.value("description").toString();
    e.startDate = QDateTime::fromString(q.value("start_date").toString(), Qt::ISODate);
    e.endDate = QDateTime::fromString(q.value("end_date").toString(), Qt::ISODate);
    e.allDay = q.value("all_day").toInt() == 1;
    e.color = stringToColor(q.value("color").toString());
    e.category = stringToCategory(q.value("category").toString());
    e.location = q.value("location").toString();
    e.reminder = q.value("reminder").toInt();
    e.priority = stringToPriority(q.value("priority").toString());
    e.source = stringToSource(q.value("source").toString());
    e.aiBatchId = q.value("ai_batch_id").toString();
    e.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    e.updatedAt = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
    return e;
}

QList<CalendarEvent> Database::getAllEvents() {
    QList<CalendarEvent> list;
    QSqlQuery q(m_db);
    q.exec("SELECT * FROM events ORDER BY start_date ASC");
    while (q.next()) list.append(rowToEvent(q));
    return list;
}

QList<CalendarEvent> Database::getEventsByRange(const QDateTime &start, const QDateTime &end) {
    QList<CalendarEvent> list;
    QSqlQuery q(m_db);
    q.prepare(R"SQL(
        SELECT * FROM events
        WHERE start_date < :end AND end_date > :start
        ORDER BY start_date ASC
    )SQL");
    q.bindValue(":end", end.toString(Qt::ISODate));
    q.bindValue(":start", start.toString(Qt::ISODate));
    if (!q.exec()) {
        qWarning() << "getEventsByRange:" << q.lastError().text();
        return list;
    }
    while (q.next()) list.append(rowToEvent(q));
    return list;
}

bool Database::insertEvent(const CalendarEvent &event) {
    QSqlQuery q(m_db);
    q.prepare(R"SQL(
        INSERT INTO events
        (id, title, description, start_date, end_date, all_day, color, category,
         location, reminder, priority, source, ai_batch_id, created_at, updated_at)
        VALUES
        (:id, :title, :description, :start_date, :end_date, :all_day, :color, :category,
         :location, :reminder, :priority, :source, :ai_batch_id, :created_at, :updated_at)
    )SQL");
    q.bindValue(":id", event.id);
    q.bindValue(":title", event.title);
    q.bindValue(":description", event.description);
    q.bindValue(":start_date", event.startDate.toString(Qt::ISODate));
    q.bindValue(":end_date", event.endDate.toString(Qt::ISODate));
    q.bindValue(":all_day", event.allDay ? 1 : 0);
    q.bindValue(":color", colorToString(event.color));
    q.bindValue(":category", categoryToString(event.category));
    q.bindValue(":location", event.location);
    q.bindValue(":reminder", event.reminder);
    q.bindValue(":priority", priorityToString(event.priority));
    q.bindValue(":source", sourceToString(event.source));
    q.bindValue(":ai_batch_id", event.aiBatchId.isEmpty() ? QVariant() : QVariant(event.aiBatchId));
    q.bindValue(":created_at", event.createdAt.toString(Qt::ISODate));
    q.bindValue(":updated_at", event.updatedAt.toString(Qt::ISODate));

    if (!q.exec()) {
        qWarning() << "insertEvent:" << q.lastError().text();
        return false;
    }
    emit eventsChanged();
    if (!event.aiBatchId.isEmpty()) emit aiBatchesChanged();
    return true;
}

bool Database::updateEvent(const CalendarEvent &event) {
    QSqlQuery q(m_db);
    q.prepare(R"SQL(
        UPDATE events SET
            title = :title,
            description = :description,
            start_date = :start_date,
            end_date = :end_date,
            all_day = :all_day,
            color = :color,
            category = :category,
            location = :location,
            reminder = :reminder,
            priority = :priority,
            updated_at = :updated_at
        WHERE id = :id
    )SQL");
    q.bindValue(":id", event.id);
    q.bindValue(":title", event.title);
    q.bindValue(":description", event.description);
    q.bindValue(":start_date", event.startDate.toString(Qt::ISODate));
    q.bindValue(":end_date", event.endDate.toString(Qt::ISODate));
    q.bindValue(":all_day", event.allDay ? 1 : 0);
    q.bindValue(":color", colorToString(event.color));
    q.bindValue(":category", categoryToString(event.category));
    q.bindValue(":location", event.location);
    q.bindValue(":reminder", event.reminder);
    q.bindValue(":priority", priorityToString(event.priority));
    q.bindValue(":updated_at", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!q.exec()) {
        qWarning() << "updateEvent:" << q.lastError().text();
        return false;
    }
    emit eventsChanged();
    return true;
}

bool Database::deleteEvent(const QString &id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM events WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "deleteEvent:" << q.lastError().text();
        return false;
    }
    emit eventsChanged();
    emit aiBatchesChanged();   // 可能影响某个 batch 的存活计数
    return true;
}

// =================== AI 批次 ===================

QString Database::createAiBatch(const QString &sourceText, const QString &sourceType) {
    QString id = generateId();
    QSqlQuery q(m_db);
    q.prepare(R"SQL(
        INSERT INTO ai_batches (id, source_text, source_type, event_count, created_at)
        VALUES (:id, :src, :type, 0, :now)
    )SQL");
    q.bindValue(":id", id);
    q.bindValue(":src", sourceText);
    q.bindValue(":type", sourceType);
    q.bindValue(":now", QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!q.exec()) {
        qWarning() << "createAiBatch:" << q.lastError().text();
        return QString();
    }
    emit aiBatchesChanged();
    return id;
}

QList<AiBatchInfo> Database::getAiBatches() {
    QList<AiBatchInfo> result;
    QSqlQuery q(m_db);
    bool ok = q.exec(R"SQL(
        SELECT b.id, b.source_text, b.source_type, b.created_at,
               (SELECT COUNT(*) FROM events e WHERE e.ai_batch_id = b.id) AS alive_count
        FROM ai_batches b
        ORDER BY b.created_at DESC
    )SQL");
    if (!ok) {
        qWarning() << "getAiBatches:" << q.lastError().text();
        return result;
    }
    while (q.next()) {
        AiBatchInfo b;
        b.id = q.value("id").toString();
        b.sourceText = q.value("source_text").toString();
        b.sourceType = q.value("source_type").toString();
        b.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        b.aliveCount = q.value("alive_count").toInt();
        b.eventCount = b.aliveCount;   // 同步显示当前存活的
        result.append(b);
    }
    return result;
}

QList<CalendarEvent> Database::getBatchEvents(const QString &batchId) {
    QList<CalendarEvent> list;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM events WHERE ai_batch_id = :b ORDER BY start_date ASC");
    q.bindValue(":b", batchId);
    if (!q.exec()) {
        qWarning() << "getBatchEvents:" << q.lastError().text();
        return list;
    }
    while (q.next()) list.append(rowToEvent(q));
    return list;
}

bool Database::deleteBatch(const QString &batchId) {
    QSqlQuery q(m_db);
    // 删事件
    q.prepare("DELETE FROM events WHERE ai_batch_id = :b");
    q.bindValue(":b", batchId);
    if (!q.exec()) {
        qWarning() << "deleteBatch events:" << q.lastError().text();
        return false;
    }
    // 删批次
    q.prepare("DELETE FROM ai_batches WHERE id = :b");
    q.bindValue(":b", batchId);
    if (!q.exec()) {
        qWarning() << "deleteBatch record:" << q.lastError().text();
        return false;
    }
    emit eventsChanged();
    emit aiBatchesChanged();
    return true;
}

bool Database::clearBatchRecord(const QString &batchId) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE events SET ai_batch_id = NULL WHERE ai_batch_id = :b");
    q.bindValue(":b", batchId);
    q.exec();
    q.prepare("DELETE FROM ai_batches WHERE id = :b");
    q.bindValue(":b", batchId);
    if (!q.exec()) return false;
    emit aiBatchesChanged();
    return true;
}

// =================== 统计 ===================

QList<CategoryStat> Database::getCategoryStats(const QDateTime &start, const QDateTime &end) {
    QList<CategoryStat> stats;
    auto events = getEventsByRange(start, end);

    QHash<EventCategory, qint64> minutes;
    QHash<EventCategory, int> counts;
    qint64 totalMin = 0;

    QDateTime now = QDateTime::currentDateTime();
    qint64 effectiveEndMs = qMin(end.toMSecsSinceEpoch(), now.toMSecsSinceEpoch());

    for (const auto &e : events) {
        qint64 lo = qMax(e.startDate.toMSecsSinceEpoch(), start.toMSecsSinceEpoch());
        qint64 hi = qMin(e.endDate.toMSecsSinceEpoch(), effectiveEndMs);
        if (hi <= lo) continue;

        qint64 mins;
        if (e.allDay) {
            mins = 0;
        } else {
            mins = (hi - lo) / 60000;
        }

        counts[e.category] += 1;
        if (mins > 0) {
            minutes[e.category] += mins;
            totalMin += mins;
        }
    }

    for (auto cat : allCategories()) {
        if (!minutes.contains(cat)) continue;
        CategoryStat s;
        s.category = cat;
        s.totalMinutes = minutes[cat];
        s.count = counts[cat];
        stats.append(s);
    }

    qint64 workMin = minutes.value(EventCategory::Work, 0);
    qint64 studyMin = minutes.value(EventCategory::Study, 0);
    qint64 restMin = totalMin - workMin - studyMin;
    if (restMin < 0) restMin = 0;

    bool hasRest = false;
    for (auto &s : stats) {
        if (s.category == EventCategory::Rest) {
            s.totalMinutes = restMin;
            hasRest = true;
            break;
        }
    }
    if (!hasRest && restMin > 0) {
        CategoryStat s;
        s.category = EventCategory::Rest;
        s.totalMinutes = restMin;
        s.count = counts.value(EventCategory::Rest, 0);
        stats.append(s);
    }

    qint64 newTotal = 0;
    for (const auto &s : stats) newTotal += s.totalMinutes;
    for (auto &s : stats) {
        s.percentage = newTotal > 0 ? (double)s.totalMinutes / newTotal * 100.0 : 0;
    }

    std::sort(stats.begin(), stats.end(), [](const CategoryStat &a, const CategoryStat &b) {
        return a.totalMinutes > b.totalMinutes;
    });
    return stats;
}

QList<DailySummary> Database::getDailySummaries(const QDateTime &start, const QDateTime &end) {
    auto events = getEventsByRange(start, end);
    QMap<QDate, qint64> minutesMap;
    QMap<QDate, int> countMap;

    QDateTime now = QDateTime::currentDateTime();
    qint64 effectiveEndMs = qMin(end.toMSecsSinceEpoch(), now.toMSecsSinceEpoch());

    for (const auto &e : events) {
        if (e.allDay) continue;

        qint64 evStartMs = qMax(e.startDate.toMSecsSinceEpoch(), start.toMSecsSinceEpoch());
        qint64 evEndMs = qMin(e.endDate.toMSecsSinceEpoch(), effectiveEndMs);
        if (evEndMs <= evStartMs) continue;

        QDate d = QDateTime::fromMSecsSinceEpoch(evStartMs).date();
        QDate endDateLimit = QDateTime::fromMSecsSinceEpoch(evEndMs - 1).date();

        while (d <= endDateLimit) {
            QDateTime dayStart(d, QTime(0, 0));
            QDateTime dayEnd(d, QTime(23, 59, 59, 999));

            qint64 overlapStart = qMax(evStartMs, dayStart.toMSecsSinceEpoch());
            qint64 overlapEnd = qMin(evEndMs, dayEnd.toMSecsSinceEpoch() + 1);

            if (overlapEnd > overlapStart) {
                minutesMap[d] += (overlapEnd - overlapStart) / 60000;
                countMap[d] += 1;
            }
            d = d.addDays(1);
        }
    }

    QList<DailySummary> result;
    QDate cur = start.date();
    QDate endDate = end.date();
    while (cur <= endDate) {
        DailySummary ds;
        ds.date = cur;
        ds.totalMinutes = minutesMap.value(cur, 0);
        ds.count = countMap.value(cur, 0);
        result.append(ds);
        cur = cur.addDays(1);
    }
    return result;
}

QList<HourlyBucket> Database::getHourlyDistribution(const QDateTime &start, const QDateTime &end) {
    auto events = getEventsByRange(start, end);
    qint64 buckets[24] = {};

    QDateTime now = QDateTime::currentDateTime();
    qint64 effectiveEndMs = qMin(end.toMSecsSinceEpoch(), now.toMSecsSinceEpoch());

    for (const auto &e : events) {
        if (e.allDay) continue;

        qint64 evStartMs = qMax(e.startDate.toMSecsSinceEpoch(), start.toMSecsSinceEpoch());
        qint64 evEndMs = qMin(e.endDate.toMSecsSinceEpoch(), effectiveEndMs);
        if (evEndMs <= evStartMs) continue;

        QDateTime curTime = QDateTime::fromMSecsSinceEpoch(evStartMs);
        while (curTime.toMSecsSinceEpoch() < evEndMs) {
            int h = curTime.time().hour();
            QDateTime hourEnd(curTime.date(), QTime(h, 59, 59, 999));
            qint64 hourEndMs = qMin(hourEnd.toMSecsSinceEpoch() + 1, evEndMs);
            qint64 segMs = hourEndMs - curTime.toMSecsSinceEpoch();
            if (segMs > 0) {
                buckets[h] += segMs / 60000;
            }
            curTime = QDateTime::fromMSecsSinceEpoch(hourEndMs);
        }
    }

    QList<HourlyBucket> result;
    for (int i = 0; i < 24; i++) {
        HourlyBucket hb;
        hb.hour = i;
        hb.totalMinutes = buckets[i];
        result.append(hb);
    }
    return result;
}

int Database::eventCountBySource(EventSource source, const QDateTime &start, const QDateTime &end) {
    QSqlQuery q(m_db);
    q.prepare(R"SQL(
        SELECT COUNT(*) FROM events
        WHERE source = :source
        AND start_date < :end AND end_date > :start
    )SQL");
    q.bindValue(":source", sourceToString(source));
    q.bindValue(":end", end.toString(Qt::ISODate));
    q.bindValue(":start", start.toString(Qt::ISODate));
    if (!q.exec()) {
        qWarning() << "eventCountBySource:" << q.lastError().text();
        return 0;
    }
    if (q.next()) return q.value(0).toInt();
    return 0;
}

} // namespace timemaster
