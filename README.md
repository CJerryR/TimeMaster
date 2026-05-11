# 时间管理大师 · Time Master

一款桌面端日程管理与时间分析工具，基于 Qt 6 + C++17 构建，使用 DeepSeek 模型把自然语言转成结构化日程。

![brand-red](https://img.shields.io/badge/brand-%23ef4444-ef4444) ![C++17](https://img.shields.io/badge/C%2B%2B-17-blue) ![Qt6](https://img.shields.io/badge/Qt-6-41cd52) ![SQLite](https://img.shields.io/badge/storage-SQLite-003B57)

---

## 核心功能

### 📅 日历
- 月 / 周 / 日 三种视图，键盘 `1` / `2` / `3` 切换，`←` `→` 翻页，`T` 回到今天。
- 6 种类别、3 个优先级、9 种颜色标签；周末自动标红。
- 当前时间线、今日高亮、事件溢出折叠为「+ N 更多」。

### ✨ AI 自然语言导入
- 顶部输入框写一句话，按 Enter：
  > 「明天下午 3 点项目评审；周三上午健身一小时；本周五出差北京」
- AI 拆出多条结构化日程，**逐条勾选**后再导入，避免误导入。
- 每次导入自动写入一个**导入批次**，可整批撤销。

### 🕐 AI 导入历史（新）
- 顶部 **🕐 导入历史** 按钮（或 `Ctrl+Z`）打开历史面板。
- 左侧：所有 AI 导入批次，按时间倒序、含原文预览和事件数。
- 右侧：当前批次的全部事件。
- 三种动作：
  - **撤销整批** — 一键删除该批次的所有事件（不可恢复）。
  - **仅清理历史记录** — 保留事件，仅清掉导入记录（用于「确认无误后归档」）。
  - **删除选中事件** — 单条删除。

### 💬 AI 对话（带日历感知）
- 默认开启「让 AI 看到我的日历」开关。
- 每次提问时，自动把**过去 7 天 + 未来 14 天**内的日程压缩成紧凑文本注入到模型上下文。
- 因此可以这样问：
  - 「我下周三有什么安排？」
  - 「这周哪天最忙？给我减负建议」
  - 「明天上午有空吗？」
- 流式输出，气泡 UI。

### 📊 统计分析
- 7 张卡片 + 图表：总时长、事件数、日均时长、最忙日子；
- 类别占比环形图、类别时长水平条；
- 每日趋势折线、节奏热力图（24×7）、来源分布、智能洞察。

### 🎨 视觉 · Warm Paper 设计语言
- **品牌色** `#D97757`（HSL 17° / 60% / 60%）— 暖橙褐，唤起信任与非侵略性。
- **底色** `#F5F2ED` — 模拟自然纸张反射率，比 `#FFFFFF` 少约 15% 蓝光刺激。
- **文字主色** `#1D1C16` — 黑里带暖棕调，对比比约 12.6:1，符合 WCAG AA。
- **几何规范** 8pt 网格基数；小组件 8px 圆角，卡片 16px，对话框 16-24px。
- **质感** 卡片采用 `rgba` 半透明叠加底层纸张渐变，配合左上 / 右下双光晕（橙 + 茶绿），层次感来自光散射而非阴影。
- 浅 / 深双主题（深色为暖调 `#262521`，非冷黑），左下角一键切换。

---

## 数据存储

```
%LOCALAPPDATA%\TimeMaster\timemaster.db
```

SQLite 单文件，备份它即可保留全部数据。

## 技术栈

| | |
|---|---|
| 语言 | C++17 |
| GUI | Qt 6 (Widgets, Network, Sql) |
| 存储 | SQLite |
| AI | DeepSeek Chat API (流式) |
| 构建 | CMake 3.16+ |
| 平台 | Windows 10/11（本包） / macOS / Linux |

## 快速编译

详见 [BUILD_WINDOWS.md](BUILD_WINDOWS.md)。简要：

```bat
build_windows.bat
```

脚本会自动找 Qt、调用 MSVC、运行 `windeployqt`，最终产物在 `build\Release\TimeMaster.exe`。

## 项目结构

```
time-master/
├── CMakeLists.txt
├── build_windows.bat         # 一键 Windows 构建
├── BUILD_WINDOWS.md          # Windows 编译指南
├── README.md
└── src/
    ├── main.cpp
    ├── core/
    │   ├── Types.{h,cpp}     # 数据模型 + 枚举（含 AiBatchInfo）
    │   ├── Database.{h,cpp}  # SQLite 持久层（含 ai_batches 表）
    │   └── DeepSeekClient.{h,cpp}  # API 客户端（含 calendar context 注入）
    └── ui/
        ├── Theme.{h,cpp}                # 双主题 + 半透明色板
        ├── MainWindow.{h,cpp}           # 顶层窗口 + 渐变背景层
        ├── Sidebar.{h,cpp}              # 左侧导航
        ├── CalendarPage.{h,cpp}         # 日历主页
        ├── AnalyticsPage.{h,cpp}        # 分析页
        ├── ChatPage.{h,cpp}             # AI 对话（含 calendar context）
        ├── AiHistoryDialog.{h,cpp}      # ★ 新：AI 导入历史
        ├── EventDialog.{h,cpp}          # 新建 / 编辑事件
        ├── SettingsDialog.{h,cpp}       # 设置
        ├── MonthView.{h,cpp} / TimeGridView.{h,cpp}  # 日历视图
        ├── CategoryPieChart.{h,cpp}
        └── widgets/
            ├── StatsCardsWidget
            ├── HorizontalBarChart
            ├── DailyTrendChart
            ├── RhythmCardWidget
            ├── SourceDistributionWidget
            └── InsightsWidget
```

## 快捷键

| 键 | 动作 |
|---|---|
| `Ctrl+N` | 新建事件 |
| `Ctrl+Z` | 打开 AI 导入历史（撤销入口） |
| `T` | 回到今天 |
| `← / →` | 上一 / 下一时间段 |
| `1 / 2 / 3` | 切到日 / 周 / 月视图 |
| `Enter`（AI 输入框） | 解析 |
| `Enter`（对话框） | 发送 |

## 许可

仅供个人学习与使用。
