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

// 日历视图模式：日视图 / 周视图 / 月视图
enum class CalendarView { Day, Week, Month };

// 事件分类：工作 / 学习 / 娱乐 / 运动 / 休息 / 社交 / 个人 / 其他
enum class EventCategory {
    Work, Study, Entertainment, Exercise,
    Rest, Social, Personal, Other
};

// 事件优先级：紧急 / 普通 / 低优先级
enum class EventPriority { Urgent, Normal, Low };

// 事件颜色枚举（共 12 种色值，亮色/暗色主题各有一套映射）
enum class EventColor {
    Red, Orange, Yellow, Green, Teal, Blue,
    Indigo, Purple, Pink, Brown, Gray, Cyan
};

// 事件来源：手动创建 / AI 解析导入 / 聊天生成
enum class EventSource { Manual, AiParse, Chat };

// 调色板结构：包含背景色、文字色、边框色和颜色名称标签
struct ColorPalette {
    QColor bg;
    QColor text;
    QColor border;
    QString label;
};

// 日历事件（单条日程），包含标题、时间、分类、颜色、优先级等信息
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

    // 计算事件持续时长（分钟），由 startDate 与 endDate 的差值得出
    qint64 durationMinutes() const {
        return startDate.secsTo(endDate) / 60;
    }
};

// AI 日程建议，由 DeepSeek 解析用户输入后生成，可手动确认转为正式事件
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

// 分类统计：按事件类别汇总的总分钟数、事件数及占比
struct CategoryStat {
    EventCategory category;
    qint64 totalMinutes = 0;
    int count = 0;
    double percentage = 0.0;
};

// 日汇总：某日的总时长（分钟）与事件数量
struct DailySummary {
    QDate date;
    qint64 totalMinutes = 0;
    int count = 0;
};

// 小时桶：按小时聚合的时长数据，用于热力图或趋势图
struct HourlyBucket {
    int hour = 0;
    qint64 totalMinutes = 0;
};

// ---- 颜色映射（亮色 + 暗色主题分别有一套） ----
// 返回亮色主题下每种 EventColor 对应的调色板（bg/text/border）
QHash<EventColor, ColorPalette> eventColorsLight();
// 返回暗色主题下每种 EventColor 对应的调色板（bg/text/border）
QHash<EventColor, ColorPalette> eventColorsDark();

// ---- 字符串转换 ----
// 将事件分类转为内部存储用的英文 key
QString categoryToString(EventCategory c);
// 将事件分类转为当前语言环境下的显示文本
QString categoryLabel(EventCategory c);
// 将英文 key 字符串解析回 EventCategory 枚举
EventCategory stringToCategory(const QString &s);

// 将事件颜色转为内部存储用的英文名称
QString colorToString(EventColor c);
// 将英文名称字符串解析回 EventColor 枚举
EventColor stringToColor(const QString &s);

// 将事件优先级转为内部存储用的英文 key
QString priorityToString(EventPriority p);
// 将事件优先级转为当前语言环境下的显示文本
QString priorityLabel(EventPriority p);
// 将英文 key 字符串解析回 EventPriority 枚举
EventPriority stringToPriority(const QString &s);

// 将事件来源转为内部存储用的英文 key
QString sourceToString(EventSource s);
// 将英文 key 字符串解析回 EventSource 枚举
EventSource stringToSource(const QString &s);

// 返回所有事件分类的列表（遍历用）
QList<EventCategory> allCategories();
// 返回所有事件颜色的列表（遍历用）
QList<EventColor> allColors();
// 返回所有事件优先级的列表（遍历用）
QList<EventPriority> allPriorities();

// 返回事件分类的默认推荐颜色
EventColor categoryDefaultColor(EventCategory c);

} // namespace timemaster

Q_DECLARE_METATYPE(timemaster::CalendarEvent)
Q_DECLARE_METATYPE(timemaster::AiBatchInfo)
Q_DECLARE_METATYPE(timemaster::ChatAction)
