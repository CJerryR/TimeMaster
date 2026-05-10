# 时智 (TimePlan) — C++ / Qt 6 版

> 智能日程管理桌面应用，使用 C++ 和 Qt 6 实现，原生编译为 macOS / Linux / Windows 可执行文件。

这是从原 Electron + React + TypeScript 版本完整重写的 C++ 版本。所有核心功能都已迁移：日历视图（日 / 周 / 月）、自然语言 AI 解析（DeepSeek）、事件管理、数据分析、AI 对话。

## ✨ 特性

### 核心功能
- 📅 **三种日历视图**：日视图、周视图、月视图无缝切换
- 🤖 **AI 自然语言解析**：在顶部输入框直接说"明天下午 3 点和 Tom 开会两小时"，自动创建日程
- 💬 **AI 对话助手**：基于 DeepSeek 的流式对话，可询问日程、获取时间管理建议
- 📊 **数据分析**：周 / 月 / 30 天三种维度的时间分配饼图与统计
- 🌓 **浅色 / 深色双主题**：完全独立设计的两套配色方案，不是简单反转
- 💾 **本地 SQLite 存储**：所有数据保存在用户目录，无需账号、无云依赖

### 相比原版的 UI/UX 优化
- ✅ **月视图溢出处理**：超过 3 个事件时显示"+N 更多"按钮（原版直接挤压）
- ✅ **键盘快捷键**：
  - `⌘N` / `Ctrl+N` 新建事件
  - `T` 跳到今天
  - `←` `→` 上 / 下一周期
  - `1` `2` `3` 切换日 / 周 / 月视图
- ✅ **AI 解析确认面板**：每条 AI 解析结果都有勾选框，可挑选导入
- ✅ **双击事件直接编辑**
- ✅ **当前时间红线**：日 / 周视图自动显示当前时间，每分钟刷新
- ✅ **重叠事件智能布局**：列贪心算法，相互重叠的事件并排显示
- ✅ **饼图重绘**：原版 SVG 改为高质量 QPainter 抗锯齿绘制，环形 + 中心总计

## 📁 项目结构

```
timeplan-cpp/
├── CMakeLists.txt           # 构建配置
├── BUILD_MAC.md             # Mac 编译详细步骤
├── README.md                # 本文件
├── src/
│   ├── main.cpp             # 入口
│   ├── core/                # 业务逻辑层（无 UI 依赖）
│   │   ├── Types.{h,cpp}    # 数据类型 (CalendarEvent, ScheduleSuggestion)
│   │   ├── Database.{h,cpp} # SQLite 封装 (QtSql)
│   │   └── DeepSeekClient.{h,cpp}  # DeepSeek API 客户端 (SSE 流式)
│   └── ui/                  # 表现层
│       ├── Theme.{h,cpp}    # 主题单例 + 全局 QSS
│       ├── Sidebar.{h,cpp}  # 左侧导航
│       ├── MainWindow.{h,cpp}      # 主窗口
│       ├── CalendarPage.{h,cpp}    # 日历页（含 AI 解析栏 + 三种视图切换）
│       ├── MonthView.{h,cpp}       # 月视图绘制
│       ├── TimeGridView.{h,cpp}    # 日 / 周视图（共用网格）
│       ├── EventDialog.{h,cpp}     # 事件创建/编辑对话框
│       ├── AnalyticsPage.{h,cpp}   # 分析页（含 PieChartWidget）
│       ├── ChatPage.{h,cpp}        # AI 对话页
│       └── SettingsDialog.{h,cpp}  # 设置（API Key / 主题）
```

## 🚀 编译

完整步骤见 [BUILD_MAC.md](./BUILD_MAC.md)。

最简单的三条命令（macOS）：

```bash
brew install qt@6 cmake
cmake -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
cmake --build build -j
open build/TimePlan.app
```

Linux 用户需要 `qt6-base-dev`，Windows 需要 Qt 官方安装器或 vcpkg。

## ⚙ 配置

首次启动后：
1. 点左下角 **⚙ 设置** 图标
2. 输入 [DeepSeek API Key](https://platform.deepseek.com/api_keys)
3. 保存即可使用 AI 功能

无 API Key 时，AI 解析与对话会提示配置，其他所有功能（手动创建事件、视图切换、分析等）正常使用。

## 💾 数据存储位置

| 平台    | 路径                                                       |
| ------- | ---------------------------------------------------------- |
| macOS   | `~/Library/Application Support/TimeplanCpp/timeplan.db`    |
| Linux   | `~/.local/share/TimeplanCpp/timeplan.db`                   |
| Windows | `%APPDATA%/TimeplanCpp/timeplan.db`                        |

API Key 通过 `QSettings` 加密存储于系统钥匙串/注册表（依赖平台默认实现）。

## 📜 License

MIT
