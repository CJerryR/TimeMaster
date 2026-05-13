//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "Types.h"
#include "I18n.h"

namespace timemaster {

// 亮色主题颜色映射表：每种颜色对应浅色背景、深色文字和高亮边框
QHash<EventColor, ColorPalette> eventColorsLight() {
    return {
        {EventColor::Red,    {QColor("#fee2e2"), QColor("#dc2626"), QColor("#fca5a5"), "红色"}},
        {EventColor::Orange, {QColor("#ffedd5"), QColor("#ea580c"), QColor("#fdba74"), "橙色"}},
        {EventColor::Yellow, {QColor("#fef9c3"), QColor("#ca8a04"), QColor("#fde047"), "黄色"}},
        {EventColor::Green,  {QColor("#dcfce7"), QColor("#16a34a"), QColor("#86efac"), "绿色"}},
        {EventColor::Teal,   {QColor("#ccfbf1"), QColor("#0d9488"), QColor("#5eead4"), "青色"}},
        {EventColor::Blue,   {QColor("#dbeafe"), QColor("#2563eb"), QColor("#93c5fd"), "蓝色"}},
        {EventColor::Indigo, {QColor("#e0e7ff"), QColor("#4f46e5"), QColor("#a5b4fc"), "靛蓝"}},
        {EventColor::Purple, {QColor("#f3e8ff"), QColor("#9333ea"), QColor("#d8b4fe"), "紫色"}},
        {EventColor::Pink,   {QColor("#fce7f3"), QColor("#db2777"), QColor("#f9a8d4"), "粉色"}},
        {EventColor::Brown,  {QColor("#fef3c7"), QColor("#92400e"), QColor("#fcd34d"), "棕色"}},
        {EventColor::Gray,   {QColor("#f3f4f6"), QColor("#6b7280"), QColor("#d1d5db"), "灰色"}},
        {EventColor::Cyan,   {QColor("#cffafe"), QColor("#0891b2"), QColor("#67e8f9"), "天蓝"}},
    };
}

// 暗色主题颜色映射表：每种颜色对应深色背景、浅色文字和高亮边框
QHash<EventColor, ColorPalette> eventColorsDark() {
    return {
        {EventColor::Red,    {QColor("#3f1a1a"), QColor("#fca5a5"), QColor("#7f1d1d"), "红色"}},
        {EventColor::Orange, {QColor("#3d2010"), QColor("#fdba74"), QColor("#7c2d12"), "橙色"}},
        {EventColor::Yellow, {QColor("#3a300d"), QColor("#fde047"), QColor("#713f12"), "黄色"}},
        {EventColor::Green,  {QColor("#0f2e1a"), QColor("#86efac"), QColor("#14532d"), "绿色"}},
        {EventColor::Teal,   {QColor("#0d2e2a"), QColor("#5eead4"), QColor("#134e4a"), "青色"}},
        {EventColor::Blue,   {QColor("#172554"), QColor("#93c5fd"), QColor("#1e3a8a"), "蓝色"}},
        {EventColor::Indigo, {QColor("#1e1b4b"), QColor("#a5b4fc"), QColor("#3730a3"), "靛蓝"}},
        {EventColor::Purple, {QColor("#2a1739"), QColor("#d8b4fe"), QColor("#581c87"), "紫色"}},
        {EventColor::Pink,   {QColor("#391323"), QColor("#f9a8d4"), QColor("#831843"), "粉色"}},
        {EventColor::Brown,  {QColor("#332010"), QColor("#fcd34d"), QColor("#78350f"), "棕色"}},
        {EventColor::Gray,   {QColor("#2a2a2e"), QColor("#9ca3af"), QColor("#4b5563"), "灰色"}},
        {EventColor::Cyan,   {QColor("#0e2a31"), QColor("#67e8f9"), QColor("#155e75"), "天蓝"}},
    };
}

// 将 EventCategory 枚举转换为持久化用的英文 key
QString categoryToString(EventCategory c) {
    switch (c) {
        case EventCategory::Work:          return "work";
        case EventCategory::Study:         return "study";
        case EventCategory::Entertainment: return "entertainment";
        case EventCategory::Exercise:      return "exercise";
        case EventCategory::Rest:          return "rest";
        case EventCategory::Social:        return "social";
        case EventCategory::Personal:      return "personal";
        case EventCategory::Other:         return "other";
    }
    return "other";
}

// 返回事件分类在当前语言（I18n）下的显示名称
QString categoryLabel(EventCategory c) {
    switch (c) {
        case EventCategory::Work:          return I18n::t("cat.work");
        case EventCategory::Study:         return I18n::t("cat.study");
        case EventCategory::Entertainment: return I18n::t("cat.entertainment");
        case EventCategory::Exercise:      return I18n::t("cat.exercise");
        case EventCategory::Rest:          return I18n::t("cat.rest");
        case EventCategory::Social:        return I18n::t("cat.social");
        case EventCategory::Personal:      return I18n::t("cat.personal");
        case EventCategory::Other:         return I18n::t("cat.other");
    }
    return I18n::t("cat.other");
}

// 将英文 key 字符串解析为 EventCategory 枚举，无法识别时返回 Other
EventCategory stringToCategory(const QString &s) {
    if (s == "work") return EventCategory::Work;
    if (s == "study") return EventCategory::Study;
    if (s == "entertainment") return EventCategory::Entertainment;
    if (s == "exercise") return EventCategory::Exercise;
    if (s == "rest") return EventCategory::Rest;
    if (s == "social") return EventCategory::Social;
    if (s == "personal") return EventCategory::Personal;
    return EventCategory::Other;
}

// 将 EventColor 枚举转换为持久化用的英文名称（按索引映射）
QString colorToString(EventColor c) {
    static const QStringList names = {
        "red","orange","yellow","green","teal","blue",
        "indigo","purple","pink","brown","gray","cyan"
    };
    int idx = static_cast<int>(c);
    if (idx < 0 || idx >= names.size()) return "blue";
    return names[idx];
}

// 将英文名称字符串解析为 EventColor 枚举，找不到时返回 Blue
EventColor stringToColor(const QString &s) {
    static const QHash<QString, EventColor> m = {
        {"red", EventColor::Red}, {"orange", EventColor::Orange},
        {"yellow", EventColor::Yellow}, {"green", EventColor::Green},
        {"teal", EventColor::Teal}, {"blue", EventColor::Blue},
        {"indigo", EventColor::Indigo}, {"purple", EventColor::Purple},
        {"pink", EventColor::Pink}, {"brown", EventColor::Brown},
        {"gray", EventColor::Gray}, {"cyan", EventColor::Cyan},
    };
    return m.value(s, EventColor::Blue);
}

// 将 EventPriority 枚举转换为持久化用的英文 key
QString priorityToString(EventPriority p) {
    switch (p) {
        case EventPriority::Urgent: return "urgent";
        case EventPriority::Normal: return "normal";
        case EventPriority::Low:    return "low";
    }
    return "normal";
}

// 返回事件优先级在当前语言（I18n）下的显示名称
QString priorityLabel(EventPriority p) {
    switch (p) {
        case EventPriority::Urgent: return I18n::t("pri.urgent");
        case EventPriority::Normal: return I18n::t("pri.normal");
        case EventPriority::Low:    return I18n::t("pri.low");
    }
    return I18n::t("pri.normal");
}

// 将英文 key 字符串解析为 EventPriority 枚举，无法识别时返回 Normal
EventPriority stringToPriority(const QString &s) {
    if (s == "urgent") return EventPriority::Urgent;
    if (s == "low")    return EventPriority::Low;
    return EventPriority::Normal;
}

// 将 EventSource 枚举转换为持久化用的英文 key
QString sourceToString(EventSource s) {
    switch (s) {
        case EventSource::Manual:  return "manual";
        case EventSource::AiParse: return "ai_parse";
        case EventSource::Chat:    return "chat";
    }
    return "manual";
}

// 将英文 key 字符串解析为 EventSource 枚举，无法识别时返回 Manual
EventSource stringToSource(const QString &s) {
    if (s == "ai_parse") return EventSource::AiParse;
    if (s == "chat")     return EventSource::Chat;
    return EventSource::Manual;
}

// 返回全部 EventCategory 枚举值的列表（按逻辑顺序排列）
QList<EventCategory> allCategories() {
    return {
        EventCategory::Work, EventCategory::Study, EventCategory::Entertainment,
        EventCategory::Exercise, EventCategory::Rest, EventCategory::Social,
        EventCategory::Personal, EventCategory::Other
    };
}

// 返回全部 EventColor 枚举值的列表（按彩虹色顺序排列）
QList<EventColor> allColors() {
    return {
        EventColor::Red, EventColor::Orange, EventColor::Yellow, EventColor::Green,
        EventColor::Teal, EventColor::Blue, EventColor::Indigo, EventColor::Purple,
        EventColor::Pink, EventColor::Brown, EventColor::Gray, EventColor::Cyan
    };
}

// 返回全部 EventPriority 枚举值的列表（紧急->普通->低优先级）
QList<EventPriority> allPriorities() {
    return { EventPriority::Urgent, EventPriority::Normal, EventPriority::Low };
}

// 为每种事件分类返回一个合适的默认颜色
EventColor categoryDefaultColor(EventCategory c) {
    switch (c) {
        case EventCategory::Work:          return EventColor::Blue;
        case EventCategory::Study:         return EventColor::Indigo;
        case EventCategory::Entertainment: return EventColor::Pink;
        case EventCategory::Exercise:      return EventColor::Green;
        case EventCategory::Rest:          return EventColor::Teal;
        case EventCategory::Social:        return EventColor::Orange;
        case EventCategory::Personal:      return EventColor::Purple;
        case EventCategory::Other:         return EventColor::Gray;
    }
    return EventColor::Gray;
}

} // namespace timemaster
