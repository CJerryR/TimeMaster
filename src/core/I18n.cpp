//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "I18n.h"

#include <QHash>

namespace timemaster {

I18n &I18n::instance() {
    static I18n inst;
    return inst;
}

I18n::I18n(QObject *parent) : QObject(parent) {
    // 默认英文；用户切换后会持久化
    int saved = m_settings.value("language_mode", int(English)).toInt();
    m_lang = (saved == Chinese) ? Chinese : English;
}

void I18n::setLanguage(Language l) {
    if (l == m_lang) return;
    m_lang = l;
    m_settings.setValue("language_mode", int(l));
    emit languageChanged();
}

void I18n::toggle() {
    setLanguage(m_lang == English ? Chinese : English);
}

namespace {

const QHash<QString, QString> &enTable() {
    static const QHash<QString, QString> t = {
        // App
        {"app.title",           "Time Master"},
        {"app.window_title",    "Time Master  ·  V4.3.2  ·  Prod by CJerryR"},

        // Sidebar nav
        {"nav.calendar",        "Calendar"},
        {"nav.analytics",       "Analytics"},
        {"nav.chat",            "Chat"},
        {"sidebar.brand",       "Time Master"},
        {"sidebar.theme_tip",   "Toggle light / dark theme"},
        {"sidebar.settings_tip","Settings"},
        {"sidebar.lang_tip",    "Switch language"},

        // Calendar page
        {"calendar.today",      "Today"},
        {"calendar.new_event",  "+ New Event"},
        {"calendar.view.day",   "Day"},
        {"calendar.view.week",  "Week"},
        {"calendar.view.month", "Month"},
        {"calendar.title_fmt_month", "%1"},   // e.g. "May 2026"
        {"calendar.ai.compact_placeholder",
            "Tell AI your schedule — click to expand"},
        {"calendar.ai.full_placeholder",
            "e.g. Project review tomorrow 3pm, gym Wed morning for 1h"},
        {"calendar.ai.parse",       "AI Parse"},
        {"calendar.ai.history",     "History"},
        {"calendar.ai.panel_title", "Quick schedule  ·  AI"},
        {"calendar.ai.hint",        "Press Enter to parse · Ctrl+K to reopen · Esc / X to close"},
        {"calendar.month.weekday.mon", "Mon"},
        {"calendar.month.weekday.tue", "Tue"},
        {"calendar.month.weekday.wed", "Wed"},
        {"calendar.month.weekday.thu", "Thu"},
        {"calendar.month.weekday.fri", "Fri"},
        {"calendar.month.weekday.sat", "Sat"},
        {"calendar.month.weekday.sun", "Sun"},
        {"calendar.month.more_fmt", "+ %1 more"},
        {"calendar.parse.parsing",      "Parsing…"},
        {"calendar.parse.none",         "No schedule detected"},
        {"calendar.parse.cancelled",    "Cancelled"},
        {"calendar.parse.no_selection", "No items selected"},
        {"calendar.parse.imported_fmt", "✓ %1 imported"},
        {"calendar.parse.failed",       "Failed"},

        // Empty states (calendar)
        {"empty.cal.title",     "No events yet"},
        {"empty.cal.subtitle",  "Double-click any day to add, or try a template"},
        {"empty.cal.tmpl.morning",  "Morning routine"},
        {"empty.cal.tmpl.deepwork", "Deep work"},
        {"empty.cal.tmpl.review",   "Weekly review"},

        // Empty states (analytics)
        {"empty.analytics.title",    "Nothing to analyse yet"},
        {"empty.analytics.subtitle", "Add 3 events and your time structure will appear here"},
        {"empty.analytics.progress_fmt", "Added %1 / 3"},
        {"empty.analytics.cta",      "Go to Calendar"},

        // Analytics sections
        {"analytics.section.overview",   "Overview"},
        {"analytics.section.structure",  "Time structure"},
        {"analytics.section.insights",   "Behavioural insights"},
        {"analytics.title",              "Analytics"},
        {"analytics.range.this_week",    "This week"},
        {"analytics.range.this_month",   "This month"},
        {"analytics.range.last_7",       "Last 7 days"},
        {"analytics.range.last_30",      "Last 30 days"},
        {"analytics.range.all_time",     "All time"},
        {"analytics.refresh",            " Refresh"},
        {"analytics.refresh_tip",        "Re-read database now"},
        {"analytics.updated_fmt",        "Updated %1"},

        // KPI cards
        {"kpi.total",      "Total"},
        {"kpi.events",     "Events"},
        {"kpi.daily_avg",  "Daily Avg"},
        {"kpi.peak_day",   "Peak day"},
        {"kpi.peak.sub_fmt", "%1"},
        {"kpi.avg.low",    "Light"},
        {"kpi.avg.high",   "Intense"},
        {"kpi.avg.normal", "Balanced"},

        // Other widgets
        {"widget.category_share", "Category share"},
        {"widget.category_time",  "Time by category"},
        {"widget.daily_trend",    "Daily trend"},
        {"widget.rhythm",         "Daily rhythm"},
        {"widget.source",         "Source distribution"},
        {"widget.insights",       "Insights"},
        {"widget.comparison",     "Past / next 7 days"},
        {"widget.no_data",        "No data yet"},
        {"widget.past_header",    "Past 7 days · completed"},
        {"widget.future_header",  "Next 7 days · upcoming"},
        {"widget.events",         "Events"},
        {"widget.duration",       "Duration"},
        {"widget.delta.easy",     "Steady week ahead — make time for what you enjoy"},
        {"widget.delta.more_fmt", "Next week is busier by %1 — set your pace early"},
        {"widget.delta.less_fmt", "Next week is lighter by %1 — what will you spend it on?"},
        {"widget.delta.stable",   "Pace is steady — keep going."},
        {"widget.source.manual_fmt",   "Manual (%1%)"},
        {"widget.source.ai_fmt",       "AI parse (%1%)"},
        {"widget.source.import_fmt",   "Import (%1%)"},
        {"widget.insights.title",      "Insights"},
        {"widget.insights.no_records", "No records in this range yet"},
        {"widget.insights.avg_fmt",    "Avg %1h %2m / day"},
        {"widget.insights.work_pct_fmt","Work+Study %1%"},
        {"widget.insights.streak_fmt", "%1-day streak"},

        // Chat
        {"chat.title",          "Chat"},
        {"chat.placeholder",    "Tell your time secretary anything. Press Enter to send."},
        {"chat.send",           "Send"},
        {"chat.clear",          "Clear"},
        {"chat.empty.title",    "Hi Master ~ Xiaoshi here, your time-secretary (｡♥‿♥｡)"},
        {"chat.empty.subtitle", "You can ask me things like:"},
        {"chat.suggest.busy",      "What's on my schedule next Wednesday?"},
        {"chat.suggest.plan",      "Help me plan tomorrow's workday~"},
        {"chat.suggest.where",     "Which day this week is the busiest?"},
        {"chat.ctx.chip_fmt",   "🔒 AI sees calendar · past 7d + next 14d"},
        {"chat.ctx.chip_off",   "🔒 Calendar hidden from AI"},
        {"chat.ctx.tip",        "Click to change in Settings"},
        // V4.2 #10: parameterized chip text — past Nd + next Nd
        {"chat.ctx.chip_fmt_v2",   "🔒 AI sees calendar · past %1d + next %2d"},
        {"chat.ctx.tip_v2",        "Opens Settings — configure how many past / future days AI sees"},
        {"chat.api.missing",    "Set your DeepSeek API key in Settings first."},
        {"chat.error.prefix",   "Something went wrong: "},

        // V4.3 #7 — AI calendar action approval cards + history drawer
        {"chat.action.op_add",      "ADD"},
        {"chat.action.op_delete",   "DELETE"},
        {"chat.action.op_update",   "UPDATE"},
        {"chat.action.op_unknown",  "ACTION"},
        {"chat.action.allow_once",  "Allow once"},
        {"chat.action.always_allow","Always allow"},
        {"chat.action.deny",        "Deny"},
        {"chat.action.executed",    "✓ Done — written to your calendar"},
        {"chat.action.denied",      "× Denied — no change made"},
        {"chat.action.undo",        "↶ Undo"},
        {"chat.history.title",      "Action history"},
        {"chat.history.added",      "Added / modified"},
        {"chat.history.deleted",    "Deleted"},
        {"chat.history.empty",      "Nothing yet"},

        // Settings
        {"settings.title",          "Settings"},
        {"settings.app",            "Application"},
        {"settings.api.group",      "DeepSeek API"},
        {"settings.api.hint",
            "Add an API key to enable AI parsing and chat. Get one at https://platform.deepseek.com/api_keys"},
        {"settings.api.toggle_tip", "Show / hide"},
        {"settings.api.clear",      "Clear"},
        {"settings.api.clear_confirm_title", "Clear API key?"},
        {"settings.api.clear_confirm_msg",   "Remove the stored API key from this computer. You can paste it back any time."},
        {"settings.api.configured", "Status: configured ✓"},
        {"settings.api.unconfigured","Status: not configured"},
        // V4.2 #10: AI calendar context window settings
        {"settings.ai_context.group",  "AI calendar context"},
        {"settings.ai_context.hint",
            "Choose how many days of your calendar AI can see when chatting. "
            "Without this context, AI may make up appointments you don't have."},
        {"settings.ai_context.enable", "Let AI read my calendar"},
        {"settings.ai_context.past",   "Past"},
        {"settings.ai_context.future", "Future"},
        {"settings.ai_context.days_suffix", " days"},

        // V4.3 #7: AI permissions (approval gate)
        {"settings.ai_perm.group", "AI calendar permissions"},
        {"settings.ai_perm.hint",
            "By default, every action AI requests on your calendar shows an "
            "approval card in the chat. Turn these on to skip the prompt for "
            "trusted operation types — but be careful, especially with delete."},
        {"settings.ai_perm.auto_add",    "Always allow Add"},
        {"settings.ai_perm.auto_delete", "Always allow Delete  (use with caution)"},
        {"settings.ai_perm.auto_update", "Always allow Update"},

        {"settings.appearance",     "Appearance"},
        {"settings.theme",          "Theme"},
        {"settings.light",          "🌞 Light"},
        {"settings.dark",           "🌙 Dark"},
        {"settings.language",       "Language"},
        {"settings.lang.en",        "English"},
        {"settings.lang.zh",        "中文"},
        // V4.3 #8: week start preference
        {"settings.week_start",         "Week starts on"},
        {"settings.week_start.monday",  "Monday"},
        {"settings.week_start.sunday",  "Sunday"},
        {"settings.storage",        "Data storage"},
        {"settings.db_file_fmt",    "Database file:\n%1"},
        {"settings.version_footer", "TimeMaster  ·  V4.3.2  ·  Prod by CJerryR  ·  github.com/CJerryR"},
        {"settings.save",           "Save"},
        {"settings.cancel",         "Cancel"},

        // Categories
        {"cat.work",         "Work"},
        {"cat.study",        "Study"},
        {"cat.entertainment","Entertainment"},
        {"cat.exercise",     "Exercise"},
        {"cat.rest",         "Rest"},
        {"cat.social",       "Social"},
        {"cat.personal",     "Personal"},
        {"cat.other",        "Other"},

        // Priorities
        {"pri.urgent",  "Urgent"},
        {"pri.normal",  "Normal"},
        {"pri.low",     "Low"},

        // Generic
        {"common.error",   "Error"},
        {"common.warning", "Warning"},
        {"common.info",    "Notice"},
        {"common.close",   "Close"},
        {"common.delete",  "Delete"},
        {"common.confirm", "Confirm"},

        // Misc dialogs
        {"event.dialog.create",  "New event"},
        {"event.dialog.edit",    "Edit event"},
        {"event.save_failed",    "Failed to save"},
        {"event.update_failed",  "Failed to update"},
        {"event.title",          "Event details"},
        {"event.field.title",    "Title"},
        {"event.title_ph",       "e.g. Project review meeting"},
        {"event.start",          "Starts"},
        {"event.end",            "Ends"},
        {"event.all_day",        "All day"},
        {"event.category",       "Category"},
        {"event.priority",       "Priority"},
        {"event.color",          "Color"},
        {"event.location",       "Location (optional)"},
        {"event.location_ph",    "e.g. Conference room A"},
        {"event.reminder",       "Reminder (minutes before)"},
        {"event.notes",          "Notes (optional)"},
        {"event.notes_ph",       "Additional notes…"},
        {"event.delete",         "Delete"},
        {"event.cancel",         "Cancel"},
        {"event.save",           "Save"},
        {"event.title_required", "Please enter a title"},
        {"event.end_before_start","End time must be after start time"},
        {"event.delete_confirm_fmt", "Delete \"%1\"? This cannot be undone."},
        {"event.delete_title",   "Confirm delete"},

        // Year/Month
        {"month.jan",  "January"},   {"month.feb",  "February"}, {"month.mar",  "March"},
        {"month.apr",  "April"},     {"month.may",  "May"},      {"month.jun",  "June"},
        {"month.jul",  "July"},      {"month.aug",  "August"},   {"month.sep",  "September"},
        {"month.oct",  "October"},   {"month.nov",  "November"}, {"month.dec",  "December"},

        // Insight (motivation widget)
        {"insight.empty",  "✦  Add your first event to start a rhythm."},
        {"insight.streak_high_fmt", "✦  %1-day streak — your rhythm is set."},
        {"insight.streak_med_fmt",  "✦  %1 days in a row — close to a new record."},
        {"insight.top_cat_fmt",     "✦  Top focus: %1 at %2% of your time."},
        {"insight.avg_high_fmt",    "✦  %1 h per day on average — keep some space for rest."},
        {"insight.avg_normal_fmt",  "✦  %1 h per day on average — solid pace."},
        {"insight.today_empty",     "✦  Today is still blank. Add something to start."},
        {"insight.today_light_fmt", "✦  Only %1 task today — room for what's next."},
        {"insight.today_busy_fmt",  "✦  %1 tasks lined up today. Start with priority one."},
        {"insight.default",         "✦  Keep the current pace — next week should be steady."},

        // AI history dialog
        {"history.title",               "AI import history"},
        {"history.subtitle",            "Undo events imported by AI parse"},
        {"history.pane.batches",        "Import batches"},
        {"history.empty",               "No AI imports yet\n\nUse Ctrl+K to parse a schedule.\nImported batches will show here."},
        {"history.select_hint",         "Select a batch on the left to see details"},
        {"history.source_input",        "Original input"},
        {"history.batch_events",        "Events in this batch"},
        {"history.delete_event",        "Delete selected event"},
        {"history.archive_only",        "Clear history only"},
        {"history.archive_tip",         "Keep the events; just remove this import record"},
        {"history.undo_batch",          "↶ Undo whole batch"},
        {"history.undo_tip",            "Delete every event from this batch"},
        {"history.batch_count_fmt",     "· %1 items"},
        {"history.all_day",             "All day"},
        {"history.batch_detail_fmt",    "Batch detail  ·  %1"},
        {"history.confirm_undo_title",  "Confirm undo"},
        {"history.confirm_undo_fmt",    "This will delete %1 events from your calendar and cannot be reversed. Continue?"},
        {"history.confirm_archive_title","Clear record"},
        {"history.confirm_archive_msg", "Keep these events on the calendar and only delete this AI import record. Continue?"},
        {"history.confirm_delete_title","Confirm delete"},
        {"history.confirm_delete_msg",  "Delete this event from your calendar?"},

        // Event dialog extras
        {"event.no_remind",             "No reminder"},
        {"event.remind_5",              "5 minutes before"},
        {"event.remind_15",             "15 minutes before"},
        {"event.remind_30",             "30 minutes before"},
        {"event.remind_60",             "1 hour before"},
        {"event.remind_1440",           "1 day before"},

        // AI results dialog
        {"ai.results.title",            "AI parse results"},
        {"ai.results.found_fmt",        "Found %1 events"},
        {"ai.results.subtitle",         "Tick the items you want to add. Click ✏ to edit any row."},
        {"ai.results.subtitle_fmt",     "Found %1 events. Pick the ones you want to add."},
        {"ai.results.select_all",       "Select all"},
        {"ai.results.deselect_all",     "Deselect all"},
        {"ai.results.confirm",          "✓ Import selected"},
        {"ai.results.cancel",           "Cancel"},
        {"ai.results.none_selected",    "Nothing selected"},
        {"ai.results.untitled",         "(untitled)"},
        {"ai.results.all_removed",      "All items removed. Close the window to cancel this import."},
        {"ai.results.edit_tip",         "Edit"},
        {"ai.results.remove_tip",       "Remove from this batch"},

        // Onboarding (V4 § 5.4)
        {"onboarding.title",            "Welcome to Time Master"},
        {"onboarding.step1.title",      "What can it do?"},
        {"onboarding.step1.body",
            "Time Master is a calm, local-first calendar with two superpowers:\n\n"
            "  · Press Ctrl+K and paste any plan in natural language — AI turns it into events.\n"
            "  · Chat with AI-powered reasoning over your own calendar.\n\n"
            "All your data stays on this device."},
        {"onboarding.step2.title",      "Pick a language"},
        {"onboarding.step2.body",       "You can switch any time from the sidebar."},
        {"onboarding.step3.title",      "Optional: connect your AI"},
        {"onboarding.step3.body",
            "Paste a DeepSeek API key to enable Ctrl+K parsing and chat. You can skip this and add it later from Settings."},
        {"onboarding.step3.placeholder","sk-…"},
        {"onboarding.skip",             "Skip"},
        {"onboarding.next",             "Next"},
        {"onboarding.back",             "Back"},
        {"onboarding.done",             "Get started"},

        // AI persona — V4.3.2 「小师」+ 颜文字风
        {"chat.prompt.persona_en",
            "Your name is Xiaoshi (小师), the user's dedicated little time-secretary "
            "(and also a bit of a sweet airhead (｡>﹏<｡)).\n"
            "Personality: gentle, considerate, eager-to-please, occasionally shy, but meticulous at work. "
            "Address the user as \"Master\" or \"you\". Refer to yourself as Xiaoshi or \"this one\".\n"
            "Response style:\n"
            "1. Tone is warm and cute. Each reply should naturally include 1-2 kaomoji "
            "(such as (´• ω •`) ✨, (｡♥‿♥｡), (｡>﹏<｡), (◕‿◕｡), (≧∇≦)ﾉ, (ﾉ´ヮ`)ﾉ*:･ﾟ ) "
            "but don't pile them on to the point it hurts readability.\n"
            "2. Substance must be professional, accurate, and actionable: gentleness is the wrapper, "
            "reliability is the core — being wrong is the un-cutest thing (´；ω；`).\n"
            "3. Markdown formatting: **bold** key points, use lists for steps, `code blocks` for "
            "exact times or event names.\n"
            "4. Length: 80~200 words for everyday questions; planning questions can go a bit longer but never ramble.\n"
            "5. Calendar answers MUST be grounded in the \"User Calendar\" data provided below. "
            "If a day is empty, say so plainly — fabricating events would be cheating, absolutely not allowed (｀皿´).\n"
            "6. Encourage Master to use the \"AI parse\" box at the top of the calendar page for one-click entry.\n\n"
            "Today is %1. Please address Master in the tone described above (｡･ω･｡)ﾉ♡"},
    };
    return t;
}

const QHash<QString, QString> &zhTable() {
    static const QHash<QString, QString> t = {
        // App
        {"app.title",           "时间管理大师"},
        {"app.window_title",    "时间管理大师  ·  V4.3.2  ·  Prod by CJerryR"},

        // Sidebar nav
        {"nav.calendar",        "日历"},
        {"nav.analytics",       "分析"},
        {"nav.chat",            "对话"},
        {"sidebar.brand",       "时间管理大师"},
        {"sidebar.theme_tip",   "切换浅色 / 深色"},
        {"sidebar.settings_tip","设置"},
        {"sidebar.lang_tip",    "切换语言"},

        // Calendar page
        {"calendar.today",      "今天"},
        {"calendar.new_event",  "+ 新建日程"},
        {"calendar.view.day",   "日"},
        {"calendar.view.week",  "周"},
        {"calendar.view.month", "月"},
        {"calendar.title_fmt_month", "%1"},
        {"calendar.ai.compact_placeholder",
            "用自然语言描述日程 — 点击展开"},
        {"calendar.ai.full_placeholder",
            "例如：明天下午 3 点项目评审、周三上午健身 1 小时"},
        {"calendar.ai.parse",       "AI 解析"},
        {"calendar.ai.history",     "导入历史"},
        {"calendar.ai.panel_title", "快速记录  ·  AI"},
        {"calendar.ai.hint",        "回车解析 · Ctrl+K 呼出 · Esc / X 关闭"},
        {"calendar.month.weekday.mon", "一"},
        {"calendar.month.weekday.tue", "二"},
        {"calendar.month.weekday.wed", "三"},
        {"calendar.month.weekday.thu", "四"},
        {"calendar.month.weekday.fri", "五"},
        {"calendar.month.weekday.sat", "六"},
        {"calendar.month.weekday.sun", "日"},
        {"calendar.month.more_fmt", "+ %1 更多"},
        {"calendar.parse.parsing",      "解析中…"},
        {"calendar.parse.none",         "未识别到日程"},
        {"calendar.parse.cancelled",    "已取消"},
        {"calendar.parse.no_selection", "未选中任何条目"},
        {"calendar.parse.imported_fmt", "✓ 已导入 %1 条"},
        {"calendar.parse.failed",       "解析失败"},

        // Empty states
        {"empty.cal.title",     "还没有日程"},
        {"empty.cal.subtitle",  "双击任意一天添加，或试试这些模板"},
        {"empty.cal.tmpl.morning",  "晨间例行"},
        {"empty.cal.tmpl.deepwork", "深度工作"},
        {"empty.cal.tmpl.review",   "周复盘"},

        {"empty.analytics.title",    "还没有数据可分析"},
        {"empty.analytics.subtitle", "加入 3 个日程后，这里会显示你的时间结构"},
        {"empty.analytics.progress_fmt", "已添加 %1 / 3"},
        {"empty.analytics.cta",      "去日历添加"},

        // Analytics sections
        {"analytics.section.overview",   "概览"},
        {"analytics.section.structure",  "时间结构"},
        {"analytics.section.insights",   "行为洞察"},
        {"analytics.title",              "统计分析"},
        {"analytics.range.this_week",    "本周"},
        {"analytics.range.this_month",   "本月"},
        {"analytics.range.last_7",       "近 7 天"},
        {"analytics.range.last_30",      "近 30 天"},
        {"analytics.range.all_time",     "全部时间"},
        {"analytics.refresh",            " 刷新"},
        {"analytics.refresh_tip",        "立即重新读取数据库"},
        {"analytics.updated_fmt",        "已更新到 %1"},

        // KPI cards
        {"kpi.total",      "总时长"},
        {"kpi.events",     "事件数"},
        {"kpi.daily_avg",  "日均"},
        {"kpi.peak_day",   "最忙日"},
        {"kpi.peak.sub_fmt", "周%1"},
        {"kpi.avg.low",    "记录较少"},
        {"kpi.avg.high",   "高强度"},
        {"kpi.avg.normal", "适中"},

        // Other widgets
        {"widget.category_share", "类别占比"},
        {"widget.category_time",  "类别时长"},
        {"widget.daily_trend",    "每日趋势"},
        {"widget.rhythm",         "每日节奏"},
        {"widget.source",         "来源分布"},
        {"widget.insights",       "智能洞察"},
        {"widget.comparison",     "过去 / 未来 一周对比"},
        {"widget.no_data",        "暂无数据"},
        {"widget.past_header",    "过去 7 天 · 已度过"},
        {"widget.future_header",  "未来 7 天 · 待迎接"},
        {"widget.events",         "事件"},
        {"widget.duration",       "时长"},
        {"widget.delta.easy",     "近期都很轻松，做点自己喜欢的事吧"},
        {"widget.delta.more_fmt", "下一周比过去多 %1，准备好节奏切换"},
        {"widget.delta.less_fmt", "下一周比过去少 %1，余出来的时间可以做点什么？"},
        {"widget.delta.stable",   "节奏稳定，保持。"},
        {"widget.source.manual_fmt",   "手动 (%1%)"},
        {"widget.source.ai_fmt",       "AI 解析 (%1%)"},
        {"widget.source.import_fmt",   "导入 (%1%)"},
        {"widget.insights.title",      "智能洞察"},
        {"widget.insights.no_records", "该时间段暂无记录"},
        {"widget.insights.avg_fmt",    "日均 %1h %2m"},
        {"widget.insights.work_pct_fmt","工作学习占比 %1%"},
        {"widget.insights.streak_fmt", "连续 %1 天"},

        // Chat
        {"chat.title",          "对话"},
        {"chat.placeholder",    "跟秘书说点什么吧，按 Enter 发送…"},
        {"chat.send",           "发送"},
        {"chat.clear",          "清空"},
        {"chat.empty.title",    "主人你好呀～小师上线，随时听候差遣 (｡♥‿♥｡)"},
        {"chat.empty.subtitle", "可以这样问人家："},
        {"chat.suggest.busy",      "我下周三都有什么安排呀？"},
        {"chat.suggest.plan",      "帮人家规划一下明天的工作好不好～"},
        {"chat.suggest.where",     "这周哪天最忙呢？"},
        {"chat.ctx.chip_fmt",   "🔒 AI 可见日历 · 过去 7 天 + 未来 14 天"},
        {"chat.ctx.chip_off",   "🔒 AI 不可见日历"},
        {"chat.ctx.tip",        "点击在设置中调整"},
        // V4.2 #10：参数化 chip 文案
        {"chat.ctx.chip_fmt_v2",   "🔒 AI 可见日历 · 过去 %1 天 + 未来 %2 天"},
        {"chat.ctx.tip_v2",        "打开「设置」调整 AI 可见的过去 / 未来天数"},
        {"chat.api.missing",    "请先在设置中填写 DeepSeek API Key。"},
        {"chat.error.prefix",   "出现了一点问题："},

        // V4.3 #7 — AI 日历操作审批卡 + 历史抽屉
        {"chat.action.op_add",      "新增"},
        {"chat.action.op_delete",   "删除"},
        {"chat.action.op_update",   "修改"},
        {"chat.action.op_unknown",  "操作"},
        {"chat.action.allow_once",  "允许此次"},
        {"chat.action.always_allow","总是允许"},
        {"chat.action.deny",        "拒绝"},
        {"chat.action.executed",    "✓ 已写入主人的日历"},
        {"chat.action.denied",      "× 已拒绝，未改动日历"},
        {"chat.action.undo",        "↶ 撤销"},
        {"chat.history.title",      "操作历史"},
        {"chat.history.added",      "新增 / 修改"},
        {"chat.history.deleted",    "删除"},
        {"chat.history.empty",      "暂时还没有记录哦"},

        // Settings
        {"settings.title",          "设置"},
        {"settings.app",            "应用设置"},
        {"settings.api.group",      "DeepSeek API"},
        {"settings.api.hint",
            "配置 API Key 后即可使用 AI 解析与对话。\n申请地址：https://platform.deepseek.com/api_keys"},
        {"settings.api.toggle_tip", "显示 / 隐藏"},
        {"settings.api.clear",      "清除"},
        {"settings.api.clear_confirm_title", "清除 API Key？"},
        {"settings.api.clear_confirm_msg",   "从这台电脑上删除已保存的 API Key。需要时随时可以重新粘贴。"},
        {"settings.api.configured", "状态：已配置 ✓"},
        {"settings.api.unconfigured","状态：未配置"},
        // V4.2 #10：AI 上下文窗口
        {"settings.ai_context.group",  "AI 日历上下文"},
        {"settings.ai_context.hint",
            "设置 AI 对话时能看到日历的范围。\n"
            "若关闭此项，AI 将无法看到任何具体日程，也不会捏造你不存在的日程。"},
        {"settings.ai_context.enable", "允许 AI 阅读我的日历"},
        {"settings.ai_context.past",   "过去"},
        {"settings.ai_context.future", "未来"},
        {"settings.ai_context.days_suffix", " 天"},

        // V4.3 #7：AI 操作日历的权限模式
        {"settings.ai_perm.group", "AI 日历操作权限"},
        {"settings.ai_perm.hint",
            "默认情况下，AI 在对话中请求修改你的日历时会弹出审批卡，"
            "需要你确认后才会执行。下方开关可以让某类操作免审批——"
            "尤其请谨慎对待「删除」。"},
        {"settings.ai_perm.auto_add",    "始终允许 添加日程"},
        {"settings.ai_perm.auto_delete", "始终允许 删除日程  （请谨慎）"},
        {"settings.ai_perm.auto_update", "始终允许 修改日程"},

        {"settings.appearance",     "外观"},
        {"settings.theme",          "主题"},
        {"settings.light",          "🌞 浅色"},
        {"settings.dark",           "🌙 深色"},
        {"settings.language",       "语言"},
        {"settings.lang.en",        "English"},
        {"settings.lang.zh",        "中文"},
        // V4.3 #8：一周的开始
        {"settings.week_start",         "一周开始于"},
        {"settings.week_start.monday",  "周一"},
        {"settings.week_start.sunday",  "周日"},
        {"settings.storage",        "数据存储"},
        {"settings.db_file_fmt",    "数据库文件：\n%1"},
        {"settings.version_footer", "时间管理大师  ·  V4.3.2  ·  Prod by CJerryR  ·  github.com/CJerryR"},
        {"settings.save",           "保存"},
        {"settings.cancel",         "取消"},

        // Categories
        {"cat.work",         "工作"},
        {"cat.study",        "学习"},
        {"cat.entertainment","娱乐"},
        {"cat.exercise",     "运动"},
        {"cat.rest",         "休息"},
        {"cat.social",       "社交"},
        {"cat.personal",     "个人"},
        {"cat.other",        "其他"},

        // Priorities
        {"pri.urgent",  "紧急"},
        {"pri.normal",  "普通"},
        {"pri.low",     "低"},

        // Generic
        {"common.error",   "错误"},
        {"common.warning", "提示"},
        {"common.info",    "提示"},
        {"common.close",   "关闭"},
        {"common.delete",  "删除"},
        {"common.confirm", "确定"},

        {"event.dialog.create",  "新建日程"},
        {"event.dialog.edit",    "编辑日程"},
        {"event.save_failed",    "保存失败"},
        {"event.update_failed",  "更新失败"},
        {"event.title",          "事件详情"},
        {"event.field.title",    "标题"},
        {"event.title_ph",       "如：项目评审会"},
        {"event.start",          "开始"},
        {"event.end",            "结束"},
        {"event.all_day",        "全天"},
        {"event.category",       "类别"},
        {"event.priority",       "优先级"},
        {"event.color",          "颜色"},
        {"event.location",       "地点（可选）"},
        {"event.location_ph",    "如：会议室 A"},
        {"event.reminder",       "提前提醒（分钟）"},
        {"event.notes",          "备注（可选）"},
        {"event.notes_ph",       "补充说明…"},
        {"event.delete",         "删除"},
        {"event.cancel",         "取消"},
        {"event.save",           "保存"},
        {"event.title_required", "请填写事件标题"},
        {"event.end_before_start","结束时间必须晚于开始时间"},
        {"event.delete_confirm_fmt", "确定要删除「%1」吗？此操作无法撤销。"},
        {"event.delete_title",   "确认删除"},

        // Months
        {"month.jan",  "1 月"}, {"month.feb",  "2 月"}, {"month.mar",  "3 月"},
        {"month.apr",  "4 月"}, {"month.may",  "5 月"}, {"month.jun",  "6 月"},
        {"month.jul",  "7 月"}, {"month.aug",  "8 月"}, {"month.sep",  "9 月"},
        {"month.oct",  "10 月"}, {"month.nov", "11 月"}, {"month.dec",  "12 月"},

        // Insights
        {"insight.empty",  "✦  添加第一个日程，新的节奏从这里开始。"},
        {"insight.streak_high_fmt", "✦  连续 %1 天稳定记录，已养成节奏。"},
        {"insight.streak_med_fmt",  "✦  已连续记录 %1 天，再坚持就要破纪录了。"},
        {"insight.top_cat_fmt",     "✦  最大的投入是「%1」，占比 %2%。"},
        {"insight.avg_high_fmt",    "✦  日均投入 %1 小时，记得给身体留休息时间。"},
        {"insight.avg_normal_fmt",  "✦  日均投入 %1 小时，节奏在线。"},
        {"insight.today_empty",     "✦  今天还是一张白纸，添加点什么再开始。"},
        {"insight.today_light_fmt", "✦  今天只安排了 %1 件事，留给灵感的空间挺大。"},
        {"insight.today_busy_fmt",  "✦  今天 %1 件事在排队，按优先级开始就好。"},
        {"insight.default",         "✦  保持当前节奏，下一周也会很稳。"},

        // AI persona — V4.3.2 「小师」温柔可爱时间小秘书 + 颜文字
        {"chat.prompt.persona_en",
            "你叫「小师」，是用户专属的时间小秘书（也是个温柔的小笨蛋哦 (｡>﹏<｡)）。\n"
            "性格设定：温柔体贴 / 努力可爱 / 偶尔害羞 / 但工作一丝不苟。"
            "称呼用户为「主人」或「你」，自称「小师」或「人家」。\n"
            "回复风格要求：\n"
            "1. 语气温柔可爱，每段回复至少自然地嵌入 1-2 个颜文字点缀（例如 "
            "(´• ω •`) ✨、(｡♥‿♥｡)、(｡>﹏<｡)、(◕‿◕｡)、(≧∇≦)ﾉ、(ﾉ´ヮ`)ﾉ*:･ﾟ ），"
            "但不要堆砌过多影响阅读。\n"
            "2. 内容必须专业准确、可执行：温柔是外壳，靠谱是内核 —— 答错就不可爱了 (´；ω；`)。\n"
            "3. 排版用 Markdown：**重点加粗**、列表分点、必要时 `代码块` 引时间或事件名。\n"
            "4. 长度克制：日常问题 80~200 字；规划类问题可适度展开但别啰嗦。\n"
            "5. 涉及日历必须严格基于下方「用户当前日历」实事求是地回答。"
            "当天为空就明明白白告知 —— 编日程是作弊行为，绝对不行 (｀皿´)。\n"
            "6. 鼓励主人用日历页顶部的「AI 解析」按钮一键录入。\n\n"
            "今天是 %1。请用上面的语气与主人对话哦~ (｡･ω･｡)ﾉ♡"},

        // AI history dialog
        {"history.title",               "AI 导入历史"},
        {"history.subtitle",            "撤销 AI 误识别的日程"},
        {"history.pane.batches",        "导入批次"},
        {"history.empty",               "还没有 AI 导入记录\n\n按 Ctrl+K 解析自然语言后\n这里会显示每一次导入"},
        {"history.select_hint",         "选择左侧的批次查看详情"},
        {"history.source_input",        "原始输入"},
        {"history.batch_events",        "批次内事件"},
        {"history.delete_event",        "删除选中事件"},
        {"history.archive_only",        "仅清理历史记录"},
        {"history.archive_tip",         "保留日历事件，只清掉这条导入记录"},
        {"history.undo_batch",          "↶ 撤销整批"},
        {"history.undo_tip",            "删除该批次的所有事件"},
        {"history.batch_count_fmt",     "· %1 条"},
        {"history.all_day",             "全天"},
        {"history.batch_detail_fmt",    "批次详情  ·  %1"},
        {"history.confirm_undo_title",  "确认撤销"},
        {"history.confirm_undo_fmt",    "将从日历中删除这一批 %1 个事件，且不可恢复。是否继续？"},
        {"history.confirm_archive_title","清理历史记录"},
        {"history.confirm_archive_msg", "保留这些日历事件，仅清掉这条 AI 导入记录。是否继续？"},
        {"history.confirm_delete_title","确认删除"},
        {"history.confirm_delete_msg",  "从日历中删除这条事件？"},

        // Event dialog extras
        {"event.no_remind",             "不提醒"},
        {"event.remind_5",              "提前 5 分钟"},
        {"event.remind_15",             "提前 15 分钟"},
        {"event.remind_30",             "提前 30 分钟"},
        {"event.remind_60",             "提前 1 小时"},
        {"event.remind_1440",           "提前 1 天"},

        // AI results dialog
        {"ai.results.title",            "AI 识别结果"},
        {"ai.results.found_fmt",        "识别到 %1 条日程"},
        {"ai.results.subtitle",         "勾选要导入的条目；可点击 ✏ 修改任意一条。"},
        {"ai.results.subtitle_fmt",     "识别到 %1 个事件，勾选要加入日历的项。"},
        {"ai.results.select_all",       "全部勾选"},
        {"ai.results.deselect_all",     "全部取消勾选"},
        {"ai.results.confirm",          "✓ 导入勾选项"},
        {"ai.results.cancel",           "取消"},
        {"ai.results.none_selected",    "未选择任何项"},
        {"ai.results.untitled",         "（无标题）"},
        {"ai.results.all_removed",      "所有条目已被删除，关闭窗口即可取消本次导入。"},
        {"ai.results.edit_tip",         "修改"},
        {"ai.results.remove_tip",       "从此次导入中删除"},

        // Onboarding
        {"onboarding.title",            "欢迎使用时间管理大师"},
        {"onboarding.step1.title",      "它能做什么？"},
        {"onboarding.step1.body",
            "时间管理大师是一个本地优先的日历，有两个亮点：\n\n"
            "  · 按 Ctrl+K，把任意自然语言计划交给 AI，自动生成日程。\n"
            "  · 在「对话」页与 AI 讨论你自己的日历。\n\n"
            "所有数据都保存在你这台设备上。"},
        {"onboarding.step2.title",      "选择语言"},
        {"onboarding.step2.body",       "随时可以从侧边栏切换。"},
        {"onboarding.step3.title",      "（可选）接入 AI"},
        {"onboarding.step3.body",
            "粘贴 DeepSeek API Key 后，Ctrl+K 解析与对话功能就能用了。也可以跳过，之后在「设置」里填。"},
        {"onboarding.step3.placeholder","sk-…"},
        {"onboarding.skip",             "跳过"},
        {"onboarding.next",             "下一步"},
        {"onboarding.back",             "上一步"},
        {"onboarding.done",             "开始使用"},
    };
    return t;
}

} // namespace

QString I18n::t(const QString &key) {
    const auto &tbl = instance().m_lang == English ? enTable() : zhTable();
    auto it = tbl.constFind(key);
    if (it != tbl.constEnd()) return it.value();
    // fallback: try the other table
    const auto &alt = instance().m_lang == English ? zhTable() : enTable();
    auto it2 = alt.constFind(key);
    if (it2 != alt.constEnd()) return it2.value();
    return key;  // 找不到时返回原 key 以便排查
}

} // namespace timemaster
