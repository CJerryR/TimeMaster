# Cadence — 分析模块完整版 + 新图标 更新包

> 这一包只覆盖分析页和图标，其他文件（Database、MainWindow、CalendarPage 等）都不动。

## 📦 包内文件

```
cadence-update/
├── src/
│   ├── assets/
│   │   └── icon.svg              ← 新图标矢量母版（1024×1024）
│   └── ui/
│       ├── AnalyticsPage.h       ← 替换原 AnalyticsPage.h
│       ├── AnalyticsPage.cpp     ← 替换原 AnalyticsPage.cpp
│       └── widgets/              ← 新增 6 个组件
│           ├── StatsCardsWidget.{h,cpp}        概览卡片 ×4 + 环比
│           ├── InsightsWidget.{h,cpp}          智能洞察列表
│           ├── HorizontalBarChart.{h,cpp}      Top 5 横向柱状图
│           ├── DailyTrendChart.{h,cpp}         每日趋势 mini 柱状图
│           ├── RhythmCardWidget.{h,cpp}        最忙日 / 高峰时段卡片
│           └── SourceDistributionWidget.{h,cpp} 手动 vs AI 占比
├── scripts/
│   └── make_icns.sh              ← 把 SVG 渲染成 .icns 的脚本
├── CMakeLists-additions.txt      ← 要合并进现有 CMakeLists 的片段
└── README.md                     ← 本文件
```

## ✅ 现在分析页有什么（对齐网页版）

| # | 模块 | 之前 | 现在 |
|---|------|------|------|
| 1 | 4 张概览卡片（总日程 / 总时长 / 日均 / 紧急）+ 环比箭头 | ❌ | ✅ |
| 2 | 智能洞察文字列表（本地推导，不调 AI） | ❌ | ✅ |
| 3 | 时间分配饼图 + 图例 | ✅ | ✅ |
| 4 | 分类耗时 Top 5 横向柱状图 | ❌ | ✅ |
| 5 | 每日趋势 mini 柱状图（带均值参考线、最高值高亮） | ❌ | ✅ |
| 6 | 最忙的一天 + 高峰时段（节奏卡片） | ❌ | ✅ |
| 7 | 录入方式分布（手动 vs AI 占比） | ❌ | ✅ |

## 🎨 新图标 / 新名字

**应用名改为 Cadence**（节奏、节拍）。理由：

- 短、好读、和「时间规划」这个产品调性贴合
- `.app` 域名好争取
- 在 macOS Launchpad 排在 C 区，邻居都是好看的应用名

**图标设计**（见 `src/assets/icon.svg`）：

- Apple 风格 squircle 圆角矩形（22.37% 圆角）
- 蓝紫渐变背景（`#5B6CFF → #7B3FE4`）
- 中央：表盘 12 / 3 / 6 / 9 主刻度 + 8 个次刻度
- 主体：金色（`#FFE7A8 → #FFB347`）从 12 点位扫向 4 点位的「节奏弧」
- 中央白底蓝心圆点表示「现在」
- 母版 1024×1024，从 16px 到 1024px 都清晰可读

## 🔧 集成步骤

### 1. 把文件拷进项目

```bash
# 在你的 timeplan-cpp 项目根目录
cp -r path/to/cadence-update/src/ui/widgets       src/ui/
cp    path/to/cadence-update/src/ui/AnalyticsPage.* src/ui/
cp -r path/to/cadence-update/src/assets           src/
cp -r path/to/cadence-update/scripts              ./
chmod +x scripts/make_icns.sh
```

### 2. 改 CMakeLists.txt

打开 `CMakeLists-additions.txt`，按里面注释把片段合进现有 CMakeLists。
关键三处：
- `project(...)` 改为 `Cadence`
- 在 `add_executable` 之前 `list(APPEND PROJECT_SOURCES …)` 加入 6 个新组件
- `if(APPLE)` 块下加图标资源

### 3. 生成图标（可选，但推荐）

```bash
./scripts/make_icns.sh
```

依赖：`brew install librsvg` 推荐，或 `brew install imagemagick`。
输出会在 `build/icon.icns`。

如果跳过这一步，应用照样能跑，只是用 Qt 默认图标。

### 4. 编译

```bash
cmake -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
open build/Cadence.app
```

## ⚠ 注意事项

### Theme 接口

新组件都通过 `cadence::Theme::instance()` 取颜色，依赖以下方法：
```cpp
QColor textPrimary();
QColor textSecondary();
QColor textPlaceholder();
QColor borderColor();
QColor bgContainer();
QColor bgComponent();
QColor brand();
```
如果你的 Theme 类方法名不同，搜替换一下即可（共 ~30 处）。

### Database 接口假设

`AnalyticsPage::rebuildStats()` 调用了：
```cpp
QList<CalendarEvent> Database::eventsInRange(const QDateTime& start,
                                              const QDateTime& end);
```
如果你之前用的是别的方法名（比如 `loadEvents` / `getEventsInRange`），把这两处调用改一下即可（在 cpp 文件里搜 `m_db->eventsInRange`）。

### CalendarEvent 字段名

实现里假设：
- `e.startDate` / `e.endDate` 是 `QDateTime`
- `e.allDay` 是 `bool`
- `e.category` / `e.priority` / `e.source` 都是 `QString`

跟之前 C++ 版本写的 `core/Types.h` 应当一致。如有差异搜替换。

### CategoryPieChart

新代码假设饼图组件叫 `cadence::CategoryPieChart` 且有 `setSlices(QVector<PieSlice>)` 接口。
如果之前的饼图类叫别的名字（比如 `PieChartWidget`），把 `AnalyticsPage.cpp` 顶部的 `#include "CategoryPieChart.h"` 和 `m_pie = new CategoryPieChart(...)` 两处改一下即可。

## 🧪 编译可能出现的小问题

我在 Linux 沙盒里没 Qt 环境，没法跑通编译来验证。基于交叉对照，**头文件、信号槽签名、Qt API 应当都对**，但第一次编译可能蹦几个小错：

1. `QButtonGroup::idClicked` — Qt 5.15+ 才有，Qt 5 早期版本要用 `buttonClicked(int)` 重载。Qt 6 没问题。
2. 某些机器上 `QList(iterator, iterator)` 构造器需要 `<QtGlobal>` 头。
3. emoji 在某些字体下显示不出来（Mac 默认会有彩色 emoji，问题不大）。

报错贴给我直接 patch。
