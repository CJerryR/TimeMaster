# 时间管理大师 · Time Master · V4.3.3

好用的日历和时间管理工具，基于 Qt 6 + C++17 构建，使用 DeepSeek 模型把自然语言转成结构化日程。

**当前版本：V4.3.3** — 见 [CHANGES_V4_3.md](CHANGES_V4_3.md) 了解 V4.3 改动；见 [CHANGES_V4_3_3.md](CHANGES_V4_3_3.md) 了解 V4.3.3 改动。

![brand-red](https://img.shields.io/badge/brand-%23D97757-D97757) ![C++17](https://img.shields.io/badge/C%2B%2B-17-blue) ![Qt6](https://img.shields.io/badge/Qt-6-41cd52) ![SQLite](https://img.shields.io/badge/storage-SQLite-003B57)



## V4.3.3 亮点

* **对话多轮记忆** AI 助手记住上下文，不再每句都像全新会话——追问"改到下午 4 点"能知道在说哪个日程。
* **语言切换即时生效** 左下角「A」/「中」按钮点击后字形立刻刷新，不再要等切主题才更新。
* **时段事件文字上移** 时间格内的日程文字整体上移 3px，底部不再被裁切遮挡。
* **AI 人设调整** 助手称呼用户为「主人」，自称「小师」。
* **66 个源文件补全中文注释** 整体可读性大幅提升。

## V4.3.2 亮点


* **赋予AI操作日历权限** 现在AI 在对话中可以增删改日程，每条变更弹审批卡，可以「允许此次 / 总是允许 / 拒绝」，所有动作进入历史抽屉，支持撤销。
* **周开始日可选** 设置里一键切换，月 / 周视图同步。
* **分析页新增刷新反馈** 默认显示全部时间数据，刷新后显示"已更新到 HH:mm:ss"。
* **EventDialog 全面修复** 颜色 12 色齐全、深色模式不再白底、类别行自动换行、文字不再重叠。
* **响应式设置面板** 底部按钮永远可见，对低分辨率屏幕友好。
* **短日程最小高度 36px** 15 分钟会议也能看到完整标题 + 时间。
* **月视图跳转到日视图** 在月视图中点击某日可以跳转到日视图。

---

## 核心功能

### 日历
- 月 / 周 / 日 三种视图，键盘 `1` / `2` / `3` 切换，`←` `→` 翻页，`T` 回到今天。
- 8 种类别、3 个优先级、**12 种颜色**标签；周末自动标红。
- 当前时间线、今日高亮、事件溢出折叠为「+ N 更多」，点击直接跳到日视图。
- 周起始日可选（周一 or 周日）。

### AI 自然语言导入
- 顶部输入框写一句话，按 Enter：
  > 「明天下午 3 点项目评审；周三上午健身一小时；本周五出差北京」
- AI 拆出多条结构化日程，**逐条勾选**后再导入，避免误导入。
- 每次导入自动写入一个**导入批次**，可整批撤销。

### AI 导入历史
- 顶部 **导入历史** 按钮（或 `Ctrl+Z`）打开历史面板。
- 三种动作：**撤销整批** / **仅清理历史记录** / **删除选中事件**。

### AI 对话（带日历感知 + 操作权限）
- 默认开启「让 AI 看到我的日历」开关。
- 每次提问时，自动把可配置范围（默认过去 7 天 + 未来 14 天）的日程注入到模型上下文。
- **V4.3 新：AI 可以请求增 / 删 / 改日程** — 每个动作弹审批卡，三个按钮：
  - 「允许此次」— 执行一次；
  - 「总是允许」— 持久化此类型自动批准（可在设置中关闭）；
  - 「拒绝」— 不入库；
- 顶栏「操作历史」抽屉两列展示最近被允许的操作，每行有 ↶ 撤销按钮。

### 📊 统计分析
- 7 张卡片 + 图表：总时长、事件数、日均时长、最忙日子；
- 类别占比环形图、类别时长水平条；
- 每日趋势折线、节奏热力图（24×7）、来源分布、智能洞察；
- 范围选择：本周 / 本月 / 近 7 天 / 近 30 天 / **全部时间（默认）**；
- 每次刷新显示"已更新到 HH:mm:ss"。

### 🎨 视觉 · Warm Paper 设计语言
- **品牌色** `#D97757`（HSL 17° / 60% / 60%）— 暖橙褐。
- **底色** `#F5F2ED` — 模拟自然纸张反射率。
- **文字主色** `#1D1C16` — 黑里带暖棕调，对比 ~12.6:1（WCAG AA）。
- 浅 / 深双主题，深色为暖调 `#262521`。

---

## 数据存储

```
%LOCALAPPDATA%\TimeMaster\timemaster.db
```

SQLite 单文件。V4.3 增加 `chat_actions` 表存储 AI 操作审计日志。

## 技术栈

|      |                                       |
| ---- | ------------------------------------- |
| 语言 | C++17                                 |
| GUI  | Qt 6 (Widgets, Network, Sql)          |
| 存储 | SQLite                                |
| AI   | DeepSeek Chat API (流式)              |
| 构建 | CMake 3.16+                           |
| 平台 | Windows 10/11（本包） / macOS / Linux |

## 快速编译

详见 [BUILD_WINDOWS.md](BUILD_WINDOWS.md)。简要：

```bat
build_windows.bat
```

## 项目结构

```
time-master/
├── CMakeLists.txt
├── README.md
├── CHANGES_V4_3.md
└── src/
    ├── main.cpp
    ├── core/
    │   ├── Types.{h,cpp}        # 数据模型 + 枚举 (+ ChatAction)
    │   ├── Database.{h,cpp}     # SQLite 持久层 (+ chat_actions 表)
    │   ├── DeepSeekClient.{h,cpp}
    │   ├── I18n.{h,cpp}         # 中英双语
    │   └── Preferences.{h,cpp}  # ★ V4.3 新：周起始 + AI 权限
    ├── utils/
    │   └── MarkdownToHtml.{h,cpp}
    └── ui/
        ├── Theme.{h,cpp}
        ├── MainWindow.{h,cpp}
        ├── Sidebar.{h,cpp}
        ├── CalendarPage.{h,cpp}
        ├── AnalyticsPage.{h,cpp}
        ├── ChatPage.{h,cpp}            # ★ V4.3:AI 审批 + 操作历史
        ├── AiHistoryDialog.{h,cpp}
        ├── AiResultsDialog.{h,cpp}
        ├── EventDialog.{h,cpp}
        ├── SettingsDialog.{h,cpp}      # ★ V4.3:滚动 + 周起始 + 权限
        ├── MonthView.{h,cpp}
        ├── TimeGridView.{h,cpp}
        ├── CategoryPieChart.{h,cpp}
        └── widgets/
            ├── StatsCardsWidget, HorizontalBarChart, DailyTrendChart
            ├── RhythmCardWidget, SourceDistributionWidget, InsightsWidget
            ├── ComparisonWidget, MotivationWidget, EmptyState
            └── FlowLayout                # ★ V4.3 新:自动换行布局
```

## 快捷键

| 键                   | 动作                         |
| -------------------- | ---------------------------- |
| `Ctrl+N`             | 新建事件                     |
| `Ctrl+Z`             | 打开 AI 导入历史（撤销入口） |
| `T`                  | 回到今天                     |
| `← / →`              | 上一 / 下一时间段            |
| `1 / 2 / 3`          | 切到日 / 周 / 月视图         |
| `Enter`（AI 输入框） | 解析                         |
| `Enter`（对话框）    | 发送                         |

## 许可

仅供个人学习与使用。

