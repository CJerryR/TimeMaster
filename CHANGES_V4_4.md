# TimeMaster V4.4 — 升级说明

发布日期：2026-05-13
基线：V4.3

V4.3 上线后用户给的 3 条进一步反馈。本轮全部修完。

---

## V4.4 #1 — 分析页主 KPI 显示 0（最严重的 bug）

**用户原话**：「我已经有日程了把，为啥分析还是没有。。。？」

**现象**：
- 选了「全部时间」范围，但顶部四个 KPI（总时长 / 事件数 / 日均 / 最忙日）全是 0；
- 同一页面下方"过去 / 未来 一周对比"显示"未来 7 天 = 4 个事件 / 4h 10m"；
- 同一份数据库，两边数字打架，明显是查询逻辑出 bug。

**根因**：`Database::getCategoryStats(start, end)` 里有这么一段（V3.x 留下的）：
```cpp
QDateTime now = QDateTime::currentDateTime();
qint64 effectiveEndMs = qMin(end.toMSecsSinceEpoch(), now.toMSecsSinceEpoch());

for (const auto &e : events) {
    qint64 lo = qMax(e.startDate.toMSecsSinceEpoch(), start.toMSecsSinceEpoch());
    qint64 hi = qMin(e.endDate.toMSecsSinceEpoch(), effectiveEndMs);
    if (hi <= lo) continue;        // ← 整条事件被跳掉

    counts[e.category] += 1;      // ← 包括事件数 +1 都执行不到
    ...
}
```

这是 V3.x 时代为了让"已度过的时长"统计上限自动停在"现在"留下的。问题是：

- 「现在」时刻之后的事件，`lo = e.startDate ≥ now`，`hi = min(end, now) = now`，因此 `hi ≤ lo`，整条 `continue`；
- 这导致**事件数也被跳过了**（V3.x 时代代码把"计数 +1"放在 `continue` 之后），同样的 bug 在 V4.3 全部时间范围下被放大。

下方 `ComparisonWidget` 没事是因为它直接调 `getEventsByRange()`、绕开了 `getCategoryStats` 那段截断，所以它能看到未来 4 个事件。

**修法**：拆成两条路径
- **事件计数**：只要事件与 `[start, end]` 有交集就 +1，不再受 `now` 影响；
- **时长统计**：未来事件按其**计划时长**计入（既然用户选了"全部时间"就是要看包含计划的总投入），不再钳到 `now`。

同样的截断在 `getDailySummaries` 和 `getHourlyDistribution` 里也有，一并清理——否则每日趋势折线和 24×7 节奏图在"未来一周"位置依然是空白。

**涉及**：`src/core/Database.cpp`（3 处统计函数都去掉 `effectiveEndMs`）

---

## V4.4 #2 — 月视图点格子直接进日视图

**用户原话**：「月视图点击日的块应该直接切日视图。」

**之前的行为**：
- 单击格子 → 没反应（事件除外，事件单击是编辑）；
- 双击空白格 → 弹"新建事件"对话框。

**改后**：
- **单击空白格**（没点到事件、没点到 +N） → 切到日视图、定位该日；
- 单击事件 → 仍然是编辑；
- 单击 +N → 仍然是切日视图（V4.3 #9 已改）；
- 双击空白格 → 也切日视图（之前是新建，现在双击和单击等效，老用户的双击肌肉记忆不会"silent fail"）；
- **"新建事件"的入口保留 3 个**：`Ctrl+N` 快捷键、右上角"+ 新建日程"按钮、日视图双击空白时间。

**涉及**：`src/ui/MonthView.cpp`（mousePressEvent 接管空白格单击）、`src/ui/CalendarPage.cpp`（onMonthDateClicked 改为 setView(Day)）

---

## V4.4 #3 — 日程块第一行文字被裁切

**现象**：截图中 "接待习近平" 的第一个字 "接" 顶部贴到了事件色块的上边缘，提手旁的横画被切掉一半。

**根因**：`TimeGridView::paintEvent` 里
```cpp
QRect tr(r.left() + 10, r.top() + 4, ...);   // 顶部 padding 只有 4px
p.drawText(QRect(tr.left(), tr.top(), tr.width(), 18), ...);  // 标题框 18px 高
```

11pt DemiBold 中文字符的可见高度差不多就是 18px，加上 antialiasing 边缘半透明的 1-2 个像素，再加上 Qt::AlignTop 让文字贴框上沿——结果就是顶部笔画"露半截"出色块边缘。

**修法**：
- 标题顶部 padding：4px → **7px**；
- 标题框高度：18px → **22px**（descender 也安全）；
- 第二行（时间）起点：`tr.top() + 18` → `tr.top() + 22`；
- 配套地，把短日程的**最小高度**从 36 提到 **44**（V4.3 #6 的最小高度需要重新算一遍：7 padding + 22 标题 + 14 时间 + 1 底部 ≈ 44）。

**涉及**：`src/ui/TimeGridView.cpp`

---

## 修改文件

```
CMakeLists.txt                  # 版本 4.3.0 → 4.3.1
installer.iss                   # 4.3 → 4.3.1 + Setup 文件名
src/core/Database.cpp           # 3 处统计修复 (#1)
src/ui/MonthView.cpp            # 单击切日视图 (#2)
src/ui/CalendarPage.cpp         # onMonthDateClicked 改语义 (#2)
src/ui/TimeGridView.cpp         # 标题 padding + 最小高度 (#3)
```

## 没改的

- 没动 UI 配色、没动布局、没动 i18n key——这一轮只修 bug + 一处 UX 改进，保持 V4.3 的整体感。
- 没动 MonthView 事件条本身（之前 `Qt::AlignVCenter` 已经居中绘制，没有 #3 同样的裁切问题）。
