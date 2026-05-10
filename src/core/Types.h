#pragma once

#include <QString>
#include <QDateTime>
#include <QColor>
#include <QMetaType>
#include <QHash>
#include <QList>

namespace timeplan {

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

// 全部分类用于循环
QList<EventCategory> allCategories();
QList<EventColor> allColors();
QList<EventPriority> allPriorities();

// 类目固定显示色，避免饼图色乱跳
EventColor categoryDefaultColor(EventCategory c);

} // namespace timeplan

Q_DECLARE_METATYPE(timeplan::CalendarEvent)
