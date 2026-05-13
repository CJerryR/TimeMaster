//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QString>
#include <QDateTime>
#include <QColor>
#include <QMetaType>
#include <QHash>
#include <QList>

namespace timemaster {

enum class CalendarView { Day, Week, Month };

enum class EventCategory {
    Work, Study, Entertainment, Exercise,
    Rest, Social, Personal, Other
};

enum class EventPriority { Urgent, Normal, Low };

enum class EventColor {
    Red, Orange, Yellow, Green, Teal, Blue,
    Indigo, Purple, Pink, Brown, Gray, Cyan
};

enum class EventSource { Manual, AiParse, Chat };

struct ColorPalette {
    QColor bg;
    QColor text;
    QColor border;
    QString label;
};

struct CalendarEvent {
    QString id;
    QString title;
    QString description;
    QDateTime startDate;
    QDateTime endDate;
    bool allDay = false;
    EventColor color = EventColor::Blue;
    EventCategory category = EventCategory::Other;
    QString location;
    int reminder = 15;
    EventPriority priority = EventPriority::Normal;
    EventSource source = EventSource::Manual;
    QString aiBatchId;        // 关联的 AI 导入批次（用于撤销）
    QDateTime createdAt;
    QDateTime updatedAt;

    qint64 durationMinutes() const {
        return startDate.secsTo(endDate) / 60;
    }
};

struct ScheduleSuggestion {
    QString title;
    QString description;
    QDateTime startDate;
    QDateTime endDate;
    int durationMinutes = 60;
    EventCategory category = EventCategory::Other;
    EventPriority priority = EventPriority::Normal;
    EventColor color = EventColor::Blue;
    bool allDay = false;
};

// AI 导入批次（一次 AI 解析对应一个批次，里面有若干事件）
struct AiBatchInfo {
    QString id;
    QString sourceText;       // 原始用户输入
    QString sourceType;       // "parse" | "chat"
    QDateTime createdAt;
    int eventCount = 0;       // 批次内事件总数（即便部分被删，仍记录原始数）
    int aliveCount = 0;       // 当前还存在的事件数
};

// V4.3 #7 — 聊天页面通过审批卡执行的一次操作（增/删/改）
// snapshotJson 存事件序列化结果，方便撤销时复原。
struct ChatAction {
    QString id;
    QString op;               // "add" | "delete" | "update"
    QString eventId;
    QString snapshotJson;     // 完整事件 JSON（用于撤销）
    QString humanSummary;     // "Added: 项目评审 · 11/15 14:00"
    QDateTime createdAt;
};

struct CategoryStat {
    EventCategory category;
    qint64 totalMinutes = 0;
    int count = 0;
    double percentage = 0.0;
};

struct DailySummary {
    QDate date;
    qint64 totalMinutes = 0;
    int count = 0;
};

struct HourlyBucket {
    int hour = 0;
    qint64 totalMinutes = 0;
};

// ---- 颜色映射（亮色 + 暗色主题分别有一套） ----
QHash<EventColor, ColorPalette> eventColorsLight();
QHash<EventColor, ColorPalette> eventColorsDark();

// ---- 字符串转换 ----
QString categoryToString(EventCategory c);
QString categoryLabel(EventCategory c);
EventCategory stringToCategory(const QString &s);

QString colorToString(EventColor c);
EventColor stringToColor(const QString &s);

QString priorityToString(EventPriority p);
QString priorityLabel(EventPriority p);
EventPriority stringToPriority(const QString &s);

QString sourceToString(EventSource s);
EventSource stringToSource(const QString &s);

QList<EventCategory> allCategories();
QList<EventColor> allColors();
QList<EventPriority> allPriorities();

EventColor categoryDefaultColor(EventCategory c);

} // namespace timemaster

Q_DECLARE_METATYPE(timemaster::CalendarEvent)
Q_DECLARE_METATYPE(timemaster::AiBatchInfo)
Q_DECLARE_METATYPE(timemaster::ChatAction)
