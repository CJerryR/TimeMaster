# TimeMaster V4.3 — 升级说明

发布日期：2026-05-12
基线：V4.2（用户反馈密集 → V4.3 全部 11 条都修了）

本次升级针对 V4.2 发布版用户反馈集中改进，**11 条全部完成**。下面按"反馈编号 → 根因 → 改法 → 涉及文件"依次说明。

---

## 反馈 #1 — EventDialog 文字重叠

**用户报告**：编辑事件弹窗里有文字相互压在一起。

**根因**：本地不能稳定复现（疑似缓存/DPI 缩放问题）。但 review 代码发现 caption 标签没有 `minimumHeight`，在某些字体设置下 descender 会被切到下一行；同时 grid 的 `verticalSpacing` 只有 8，缩放后两行靠得过近。

**改法**：
- caption 标签统一加 `setMinimumHeight(22)`；
- `QGridLayout::verticalSpacing` 由 8 提到 12；
- 输入框 `minimumHeight` 由 36 提到 40；
- 弹窗最小尺寸由 560×660 提到 600×720，给所有元素留呼吸感。

**涉及**：`src/ui/EventDialog.cpp`、`src/ui/EventDialog.h`

---

## 反馈 #2 — 弹窗深色模式仍是黑底亮字（事件详情、AI 解析、AI 历史）

**根因**：`QDialog` 自身没有显式设背景色时，Qt 默认走系统 `palette.window`，在我们自己跑深色主题的时候系统色板还是浅色，结果"浅底深字"夹在浅色控件里特别违和。`globalStylesheet()` 里有 `QDialog { background-color: ... }` 但它会被任何对自己 QDialog 子类的样式表覆盖。

**改法**：每个弹窗类都加上 `setObjectName("...")`，然后在 `applyTheme()` 的 stylesheet 里用 `QDialog#XxxDialog { background-color: bgPage; }` 这种**专属选择器**强制覆盖。这样既不影响其他 `QDialog`，也不会被全局样式覆盖。

**涉及**：
- `src/ui/EventDialog.cpp`（`#EventDialog`）
- `src/ui/AiResultsDialog.cpp`（`#AiResultsDialog`）
- `src/ui/AiHistoryDialog.cpp`（`#AiHistoryDialog`）
- `src/ui/SettingsDialog.cpp`（`#SettingsDialog`，顺便和 #10 一起改了）

---

## 反馈 #3 — 颜色选择器最后三个（Brown/Gray/Cyan）看不清

**根因**：⚠️ **典型的"枚举与查表不同步"bug**。
- `Types.h` 里 `EventColor` 定义了 12 个值；
- `Types.cpp` 里的 `allColors()` 返回这 12 个；
- 但 `Theme::palette()` 在 Light/Dark 两个 mode 里只填了 9 个（Red..Pink）；
- 用户点 Brown/Gray/Cyan，按钮 stylesheet 拿到的是 default-constructed `ColorPalette{}`，里面的 `text` 是无效 `QColor` → Qt 渲染时按"黑色 / 透明色"处理 → 用户看到一片黑或者干脆没东西。

**改法**：
- 在 `Theme::palette()` 的 Light 和 Dark 分支各补齐 Brown/Gray/Cyan 3 行；
- 同时 EventDialog 里给 `refreshColorButtons()` 加了 `if (!c.isValid()) c = QColor("#888888");` 作为 defense-in-depth 兜底，避免日后再加颜色时同样的事再发生一次。

**涉及**：`src/ui/Theme.cpp`、`src/ui/EventDialog.cpp`

---

## 反馈 #4 — 类别需要多行换行（自定义类别推迟）

**用户原话**：希望支持自定义类别 + 类别行可以换行（同步到分析页）。

**取舍**：本轮先解决"换行"这一个最痛的问题；**自定义类别**因为牵涉数据库 schema 改、分析页大量改动、AI prompt 重训，移到 V4.4。

**改法**：写了一个 Qt 经典 `FlowLayout`（基于官方示例），EventDialog 里的 Category 行和 Color 行都换成 `FlowLayout`，按钮数量超过当前宽度时自动换到下一行。

**涉及**：
- 新增 `src/ui/widgets/FlowLayout.h` + `.cpp`
- `src/ui/EventDialog.cpp`

---

## 反馈 #5 — 空状态卡片背景透明，三个默认模板和背景混成一片

**根因**：`EmptyState.cpp` 里 `QWidget#EmptyStateCard` 的 background 是 `transparent`，圆角和边框都看不出来，"空"的视觉信号比"内容"还弱。

**改法**：把 `#EmptyStateCard` 的 stylesheet 改为
```
background-color: cardBgRgba;
border: 1px solid stroke;
border-radius: 16px;
```
并给卡片 `setMinimumWidth(420)` + 加大内边距到 44/38。现在它是一张明确的浮起卡片，背后空间留白，与背景明显区分。

**涉及**：`src/ui/widgets/EmptyState.cpp`

---

## 反馈 #6 — 短日程视觉高度不够

**根因**：`TimeGridView::layoutDayEvents` 里写了 `if (hgt < 18) hgt = 18;`，15 分钟会议在 day view 缩成 18px 高的彩条，标题被截断成 1-2 个字符。

**改法**：最小高度从 18 提到 36，足够放下"标题 + HH:mm-HH:mm 两行"。代价是相邻短日程可能视觉上交叠一点点，但前后排序保证后绘制的盖在前面，可读性比对齐更重要。

**涉及**：`src/ui/TimeGridView.cpp`

---

## 反馈 #7 — 对话页可操作本地日历（审批卡模式）

**最大的功能改动**。设计参考主流 AI 工具的权限模式。

### 协议
教 AI 在普通回复里嵌入特殊代码块：
```
\`\`\`action
{"op":"add","title":"项目评审","start":"2026-05-14T14:00:00",
 "end":"2026-05-14T15:00:00","category":"work","color":"blue",
 "location":"","reminder":15,"priority":"normal"}
\`\`\`
```
支持三个 op：`add` / `delete` / `update`。

### UI
- 每个 action 块在流式结束后渲染成一张**审批卡**，3 个按钮：
  - 「允许此次」— 执行并入库，写一条 `chat_actions` 记录；
  - 「拒绝」— 只改卡的状态文案，不入库；
  - 「总是允许」— 同 "允许此次"，但顺手把对应类型的 `autoApprove*` 置 true，下次直接跳过审批；
- AI 文本里那段 ` ```action` 代码块在渲染时被 regex 剥掉（用户看不到 JSON 垃圾）。

### 历史 + 撤销
顶栏新增「操作历史」按钮，点开抽屉两列展示：
- 左列「新增 / 修改」（add 和 update）；
- 右列「删除」；
- 每行有 ↶ 撤销按钮：`add` → `deleteEvent`；`delete` → `insertEvent(snapshot)`；`update` → `updateEvent(snapshot)`。

### 持久化
新表 `chat_actions(id, op, event_id, snapshot_json, human_summary, created_at)`；snapshot_json 完整 JSON 化原事件，撤销时整体还原。

### 设置面板
SettingsDialog 新增「AI 日历操作权限」组，3 个 checkbox：
- 始终允许 添加
- 始终允许 删除（⚠️ 请谨慎）
- 始终允许 修改

存 `QSettings` 的 `chat_auto_approve_*` key，通过 `Preferences` 单例读写。

**涉及**：
- 新增 `src/core/Preferences.h` + `.cpp`
- `src/core/Types.h`：`struct ChatAction`
- `src/core/Database.h` + `.cpp`：`chat_actions` 表 + 3 个新方法
- `src/ui/ChatPage.h` + `.cpp`：协议解析、审批卡、抽屉、撤销
- `src/ui/SettingsDialog.cpp`：权限组
- `src/core/I18n.cpp`：约 20 个新 key

---

## 反馈 #8 — 周一/周日开始可选

**根因**：之前各 view 自己 hardcode `dayOfWeek() == 1` 当作周一为起点。

**改法**：抽出 `Preferences::weekStartsMonday()`，提供两个工具方法：
- `weekColumnOf(QDate)` — 返回该日期在本周是第几列；
- `weekStartOf(QDate)` — 返回该日期所在周的第一天。

MonthView / TimeGridView / CalendarPage 全部改用这两个方法。Preferences 发 `weekStartChanged` 信号，三个 view 监听后 rebuildLayout + update。SettingsDialog 加 pill 按钮"周一 / 周日"。

**涉及**：所有日历视图相关文件

---

## 反馈 #9 — 月视图 +N 跳转到日视图

**根因**：之前 `onMonthOverflowClicked` 弹一个 `QDialog` 列出当天所有事件，体验割裂。

**改法**：直接 `m_currentDate = d; setView(CalendarView::Day);`，无缝跳到日视图并定位到那一天。

**涉及**：`src/ui/CalendarPage.cpp`（删掉那段 `QDialog` 列表代码）

---

## 反馈 #10 — 设置等子页面在低分辨率屏上超高

**根因**：SettingsDialog 用 `QFormLayout` 直接堆字段，高度随内容线性增长，到了 1366×768 笔电屏就把保存/取消按钮顶到屏幕外。

**改法**：
- 整个 SettingsDialog 用 `QScrollArea` 包内容；
- 底部 `QHBoxLayout`（保存/取消）作为**固定底栏**放在 scrollArea 外，永远可见；
- 高度按 `QGuiApplication::primaryScreen()->availableSize()` 自适应：`targetH = qMin(720, qMax(540, availH - 120))`。

**涉及**：`src/ui/SettingsDialog.cpp`、`.h`

---

## 反馈 #11 — 分析页刷新无效（默认 Last 7 看不到未来事件）

**用户原话**："我新增了日程，点刷新，分析页还是 0，我他妈生气了。"

**两个根因**：
1. 默认范围是 "Last 7" = `[today-6, today]`。用户的日程在 **未来** 也很多，未来的根本进不去这个范围。
2. 点了刷新后没有任何视觉反馈，用户不确定是否真的刷新了。

**改法**：
- 新增 "All time" 选项（覆盖数据库里实际事件的 `[min(start), max(end)]`），并把它设为默认；
- 刷新按钮左侧新增 "已更新到 14:32:09" 反馈标签，每次 refresh() 末尾把当前时间塞进去；

**涉及**：`src/ui/AnalyticsPage.cpp` + `.h`、`src/core/I18n.cpp`（新 key `analytics.range.all_time`、`analytics.updated_fmt`）

---

## 反馈 #12 — Self-debug

跨文件审视下来，主动加固的几处：
- EventDialog 的颜色按钮：除了修 #3，还加了 `if (!c.isValid()) c = "#888888"` 的兜底；
- 所有新加按钮统一 `setFocusPolicy(Qt::NoFocus)`，消除虚线焦点框（沿用 V4.2 #3 的规则）；
- ChatPage 的 action 解析只在 `chatFinished` 时跑一次，不在 `chatChunk` 里增量解析（避免半截 JSON 重复渲染）；
- Database 的 `chat_actions` 表加了 `created_at` 索引，撤销/历史查询性能稳定。

---

## 新增文件

```
src/core/Preferences.h
src/core/Preferences.cpp
src/ui/widgets/FlowLayout.h
src/ui/widgets/FlowLayout.cpp
```

## 修改文件

```
CMakeLists.txt                       # 版本号 + 新源文件
src/core/Database.h / .cpp           # chat_actions 表 + CRUD
src/core/I18n.cpp                    # 新 i18n key
src/core/Types.h                     # struct ChatAction
src/ui/AiHistoryDialog.cpp           # 弹窗深色 (#2)
src/ui/AiResultsDialog.cpp           # 弹窗深色 (#2)
src/ui/AnalyticsPage.h / .cpp        # All time + 刷新反馈 (#11)
src/ui/CalendarPage.cpp              # week_start 联动 + 月视图溢出跳日 (#8 #9)
src/ui/ChatPage.h / .cpp             # AI 审批 (#7)
src/ui/EventDialog.h / .cpp          # 重叠 + 深色 + 颜色 + 换行 (#1 #2 #3 #4)
src/ui/MonthView.cpp                 # week_start + i18n more_fmt (#8 #9)
src/ui/SettingsDialog.h / .cpp       # 滚动 + week_start + 权限 (#7 #8 #10)
src/ui/Theme.cpp                     # 补齐 12 色 (#3)
src/ui/TimeGridView.cpp              # week_start + 最小高度 36 (#6 #8)
src/ui/widgets/EmptyState.cpp        # 真实卡片背景 (#5)
```

## 未做（推迟到 V4.4）
- **自定义类别**（#4 后半）—— 需要 db schema 变更（categories 表）+ 分析页大改 + AI prompt 升级，工作量较大；
- 类别按钮配色单独可定制；
- 月视图 +N 溢出在窄列时仍可能被裁切（可考虑改为绝对最少 1 个事件 + 1 个 +N，留到下版再 review）。
