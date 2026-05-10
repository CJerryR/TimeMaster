#include "AnalyticsPage.h"

#include "Theme.h"
#include "CategoryPieChart.h"
#include "widgets/StatsCardsWidget.h"
#include "widgets/InsightsWidget.h"
#include "widgets/HorizontalBarChart.h"
#include "widgets/DailyTrendChart.h"
#include "widgets/RhythmCardWidget.h"
#include "widgets/SourceDistributionWidget.h"

#include "../core/Database.h"
#include "../core/Types.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QScrollArea>
#include <QDateTime>
#include <QFrame>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QLayout>
#include <QLayoutItem>
#include <algorithm>
#include <cmath>

namespace cadence {

// ====== 类目固定颜色映射 ======
static const QHash<QString, QString> kCategoryColorMap = {
    {"work", "blue"},
    {"study", "indigo"},
    {"entertainment", "pink"},
    {"exercise", "green"},
    {"rest", "teal"},
    {"social", "orange"},
    {"personal", "purple"},
    {"other", "gray"},
};

static const QHash<QString, QString> kCategoryLabels = {
    {"work", "工作"}, {"study", "学习"}, {"entertainment", "娱乐"},
    {"exercise", "运动"}, {"rest", "休息"}, {"social", "社交"},
    {"personal", "个人"}, {"other", "其他"},
};

// 颜色字符串到 QColor 的映射(text 主色)
static QColor colorFromName(const QString& name) {
    static const QHash<QString, QString> map = {
        {"red", "#dc2626"}, {"orange", "#ea580c"}, {"yellow", "#ca8a04"},
        {"green", "#16a34a"}, {"teal", "#0d9488"}, {"blue", "#2563eb"},
        {"indigo", "#4f46e5"}, {"purple", "#9333ea"}, {"pink", "#db2777"},
        {"brown", "#92400e"}, {"gray", "#6b7280"}, {"cyan", "#0891b2"},
    };
    return QColor(map.value(name, "#6b7280"));
}

// =============================================================
// AnalyticsPage 实现
// =============================================================

AnalyticsPage::AnalyticsPage(Database* db, QWidget* parent)
    : QWidget(parent), m_db(db) {
    buildUi();
    rebuildStats();
}

void AnalyticsPage::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // -------- 顶部标题栏 --------
    auto* header = new QWidget(this);
    auto* hl = new QHBoxLayout(header);
    hl->setContentsMargins(24, 20, 24, 12);

    auto* title = new QLabel("📊 时间分析", header);
    QFont tf = title->font();
    tf.setPixelSize(20);
    tf.setWeight(QFont::Bold);
    title->setFont(tf);
    hl->addWidget(title);

    m_refreshBtn = new QPushButton("🔄 刷新", header);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(
        "QPushButton { padding: 5px 12px; border: 1px solid palette(mid); "
        "border-radius: 6px; font-size: 12px; }"
        "QPushButton:hover { border-color: palette(highlight); }");
    connect(m_refreshBtn, &QPushButton::clicked, this, &AnalyticsPage::refresh);
    hl->addWidget(m_refreshBtn);

    hl->addStretch();

    // 期间切换 segmented control
    auto* segmented = new QWidget(header);
    auto* sl = new QHBoxLayout(segmented);
    sl->setContentsMargins(2, 2, 2, 2);
    sl->setSpacing(2);
    segmented->setStyleSheet(
        "QWidget { background: palette(alternate-base); border: 1px solid palette(mid); "
        "border-radius: 8px; }"
        "QPushButton { border: none; padding: 6px 20px; border-radius: 6px; "
        "font-size: 13px; background: transparent; }"
        "QPushButton:checked { background: palette(highlight); color: white; font-weight: 600; }");
    auto* btnWeek = new QPushButton("近 7 天", segmented);
    auto* btnMonth = new QPushButton("近 30 天", segmented);
    btnWeek->setCheckable(true);
    btnMonth->setCheckable(true);
    btnWeek->setChecked(true);
    btnWeek->setCursor(Qt::PointingHandCursor);
    btnMonth->setCursor(Qt::PointingHandCursor);
    sl->addWidget(btnWeek);
    sl->addWidget(btnMonth);

    m_periodGroup = new QButtonGroup(this);
    m_periodGroup->addButton(btnWeek, WeekPeriod);
    m_periodGroup->addButton(btnMonth, MonthPeriod);
    m_periodGroup->setExclusive(true);
    connect(m_periodGroup, &QButtonGroup::idClicked,
            this, &AnalyticsPage::onPeriodChanged);
    hl->addWidget(segmented);

    root->addWidget(header);

    // -------- 滚动内容 --------
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new QWidget;
    auto* cl = new QVBoxLayout(content);
    cl->setContentsMargins(24, 12, 24, 24);
    cl->setSpacing(20);

    // 1) 概览卡片
    m_statsCards = new StatsCardsWidget(content);
    cl->addWidget(m_statsCards);

    // 2) 智能洞察
    m_insights = new InsightsWidget(content);
    cl->addWidget(m_insights);

    // 3) 时间分配 + Top 5（左右两栏）
    auto* row3 = new QWidget(content);
    auto* row3l = new QHBoxLayout(row3);
    row3l->setContentsMargins(0, 0, 0, 0);
    row3l->setSpacing(16);

    // 左：饼图 + 图例
    auto* pieCard = new QFrame(row3);
    pieCard->setObjectName("analysisCard");
    pieCard->setStyleSheet(
        "QFrame#analysisCard { background: palette(base); border: 1px solid palette(mid); "
        "border-radius: 12px; }");
    auto* pieLayout = new QVBoxLayout(pieCard);
    pieLayout->setContentsMargins(24, 24, 24, 24);
    pieLayout->setSpacing(12);
    auto* pieTitle = new QLabel("时间分配", pieCard);
    QFont pt = pieTitle->font(); pt.setPixelSize(15); pt.setWeight(QFont::DemiBold);
    pieTitle->setFont(pt);
    pieLayout->addWidget(pieTitle);

    m_pie = new CategoryPieChart(pieCard);
    m_pie->setMinimumSize(220, 220);
    auto* pieRow = new QHBoxLayout;
    pieRow->addStretch();
    pieRow->addWidget(m_pie);
    pieRow->addStretch();
    pieLayout->addLayout(pieRow);

    m_pieLegend = new QWidget(pieCard);
    auto* legendLayout = new QVBoxLayout(m_pieLegend);
    legendLayout->setContentsMargins(0, 0, 0, 0);
    legendLayout->setSpacing(6);
    pieLayout->addWidget(m_pieLegend);
    row3l->addWidget(pieCard, 1);

    // 右：分类耗时 Top 5
    auto* barCard = new QFrame(row3);
    barCard->setObjectName("analysisCard");
    barCard->setStyleSheet(
        "QFrame#analysisCard { background: palette(base); border: 1px solid palette(mid); "
        "border-radius: 12px; }");
    auto* barLayout = new QVBoxLayout(barCard);
    barLayout->setContentsMargins(24, 24, 24, 24);
    barLayout->setSpacing(12);
    auto* barTitle = new QLabel("分类耗时对比 (Top 5)", barCard);
    barTitle->setFont(pt);
    barLayout->addWidget(barTitle);
    m_barChart = new HorizontalBarChart(barCard);
    barLayout->addWidget(m_barChart, 1);
    row3l->addWidget(barCard, 1);

    cl->addWidget(row3);

    // 4) 每日趋势
    auto* trendCard = new QFrame(content);
    trendCard->setObjectName("analysisCard");
    trendCard->setStyleSheet(
        "QFrame#analysisCard { background: palette(base); border: 1px solid palette(mid); "
        "border-radius: 12px; }");
    auto* trendLayout = new QVBoxLayout(trendCard);
    trendLayout->setContentsMargins(24, 24, 24, 24);
    trendLayout->setSpacing(12);
    auto* trendTitle = new QLabel("每日日程趋势", trendCard);
    trendTitle->setFont(pt);
    trendLayout->addWidget(trendTitle);
    m_trendChart = new DailyTrendChart(trendCard);
    trendLayout->addWidget(m_trendChart);
    cl->addWidget(trendCard);

    // 5) 节奏卡片（最忙日 + 高峰时段）
    auto* row5 = new QWidget(content);
    auto* row5l = new QHBoxLayout(row5);
    row5l->setContentsMargins(0, 0, 0, 0);
    row5l->setSpacing(16);
    m_busiestCard = new RhythmCardWidget("最忙的一天", row5);
    m_peakHourCard = new RhythmCardWidget("高峰时段", row5);
    row5l->addWidget(m_busiestCard, 1);
    row5l->addWidget(m_peakHourCard, 1);
    cl->addWidget(row5);

    // 6) 录入方式分布
    m_sourceWidget = new SourceDistributionWidget(content);
    cl->addWidget(m_sourceWidget);

    cl->addStretch();
    m_scroll->setWidget(content);
    root->addWidget(m_scroll, 1);
}

void AnalyticsPage::onPeriodChanged(int periodId) {
    m_period = static_cast<Period>(periodId);
    rebuildStats();
}

void AnalyticsPage::refresh() {
    rebuildStats();
}

// =============================================================
// 工具函数
// =============================================================

double AnalyticsPage::clampedMinutes(const CalendarEvent& e,
                                      const QDateTime& periodStart,
                                      const QDateTime& periodEnd,
                                      const QDateTime& now) {
    qint64 start = e.startDate.toMSecsSinceEpoch();
    qint64 end = e.endDate.toMSecsSinceEpoch();
    qint64 ps = periodStart.toMSecsSinceEpoch();
    qint64 pe = std::min(periodEnd.toMSecsSinceEpoch(), now.toMSecsSinceEpoch());
    qint64 lo = std::max(start, ps);
    qint64 hi = std::min(end, pe);
    if (hi <= lo) return 0;

    if (e.allDay) {
        qint64 days = std::max<qint64>(1, (hi - lo + 24LL * 3600 * 1000 - 1)
                                              / (24LL * 3600 * 1000));
        return days * 8 * 60.0;
    }
    return (hi - lo) / 60000.0;
}

QString AnalyticsPage::formatMinutes(double m) {
    if (m <= 0) return "0m";
    if (m < 60) return QString("%1m").arg((int)std::round(m));
    int h = (int)(m / 60);
    int mins = (int)std::round(m - h * 60);
    if (mins == 60) { ++h; mins = 0; }
    if (mins == 0) return QString("%1h").arg(h);
    return QString("%1h %2m").arg(h).arg(mins);
}

AnalyticsPage::Delta AnalyticsPage::makeDelta(double curr, double prev) {
    Delta d;
    if (prev <= 0 && curr <= 0) return d;
    if (prev <= 0) {
        d.text = "新增"; d.color = "#16a34a"; d.valid = true; return d;
    }
    double diff = ((curr - prev) / prev) * 100.0;
    if (std::fabs(diff) < 1.0) {
        d.text = "持平"; d.color = "#999999"; d.valid = true; return d;
    }
    d.valid = true;
    if (diff > 0) {
        d.text = QString("↑ %1%").arg((int)std::round(std::fabs(diff)));
        d.color = "#dc2626"; // 增加为红（更繁忙）
    } else {
        d.text = QString("↓ %1%").arg((int)std::round(std::fabs(diff)));
        d.color = "#16a34a"; // 减少为绿（更轻松）
    }
    return d;
}

// =============================================================
// 核心：拉取事件 + 计算所有统计 + 喂给各组件
// =============================================================

void AnalyticsPage::rebuildStats() {
    // ---- 期间界限 ----
    const QDateTime now = QDateTime::currentDateTime();
    const int days = (m_period == WeekPeriod) ? 7 : 30;
    const QDateTime periodEnd(now.date().addDays(1).startOfDay());
    const QDateTime periodStart(now.date().addDays(-(days - 1)).startOfDay());

    // 上一周期
    const QDateTime prevEnd = periodStart;
    const QDateTime prevStart = periodStart.addDays(-days);

    // ---- 事件 ----
    QList<CalendarEvent> events = m_db->eventsInRange(periodStart, periodEnd);
    QList<CalendarEvent> prevEvents = m_db->eventsInRange(prevStart, prevEnd);

    // ---- 类目时长统计 ----
    QHash<QString, double> catMinutes;
    for (const auto& e : events) {
        catMinutes[e.category] += clampedMinutes(e, periodStart, periodEnd, now);
    }

    // 总时长（所有清醒时间，包括 rest 用于饼图分母）
    double totalMinutes = 0;
    for (auto it = catMinutes.begin(); it != catMinutes.end(); ++it) totalMinutes += it.value();

    // 已安排时间（不含 rest，用于卡片）
    double nonRestMinutes = totalMinutes - catMinutes.value("rest", 0.0);
    double prevNonRestMinutes = 0;
    {
        QHash<QString, double> prevCat;
        for (const auto& e : prevEvents) {
            prevCat[e.category] += clampedMinutes(e, prevStart, prevEnd, now);
        }
        for (auto it = prevCat.begin(); it != prevCat.end(); ++it) {
            if (it.key() != "rest") prevNonRestMinutes += it.value();
        }
    }

    // 排序后的类目列表（用于饼图、Top 5、图例）
    struct CatEntry { QString cat; QString label; double mins; double pct; QString color; };
    QList<CatEntry> sortedStats;
    for (auto it = catMinutes.begin(); it != catMinutes.end(); ++it) {
        if (it.value() <= 0) continue;
        CatEntry ce;
        ce.cat = it.key();
        ce.label = kCategoryLabels.value(it.key(), it.key());
        ce.mins = it.value();
        ce.pct = (totalMinutes > 0) ? (it.value() / totalMinutes * 100.0) : 0;
        ce.color = kCategoryColorMap.value(it.key(), "gray");
        sortedStats.append(ce);
    }
    std::sort(sortedStats.begin(), sortedStats.end(),
              [](const CatEntry& a, const CatEntry& b) { return a.mins > b.mins; });

    // 每日趋势 + 最忙日
    QHash<QString, int> dailyMap;
    {
        QDate cur = periodStart.date();
        QDate endDay = periodEnd.date().addDays(-1);
        while (cur <= endDay) {
            dailyMap[cur.toString(Qt::ISODate)] = 0;
            cur = cur.addDays(1);
        }
    }
    for (const auto& e : events) {
        QString k = e.startDate.date().toString(Qt::ISODate);
        if (dailyMap.contains(k)) dailyMap[k] += 1;
    }
    QList<TrendPoint> trend;
    QStringList dailyKeys = dailyMap.keys();
    std::sort(dailyKeys.begin(), dailyKeys.end());
    int busiestCount = 0;
    QString busiestDateKey;
    for (const auto& k : dailyKeys) {
        TrendPoint tp;
        tp.date = QDate::fromString(k, Qt::ISODate);
        tp.count = dailyMap[k];
        trend.append(tp);
        if (tp.count > busiestCount) { busiestCount = tp.count; busiestDateKey = k; }
    }

    // 高峰时段（按 startHour 计数）
    QVector<int> hourBuckets(24, 0);
    for (const auto& e : events) {
        if (e.allDay) continue;
        int h = e.startDate.time().hour();
        if (h >= 0 && h < 24) hourBuckets[h] += 1;
    }
    int peakHourIdx = -1, peakHourCount = 0;
    for (int i = 0; i < 24; ++i) {
        if (hourBuckets[i] > peakHourCount) {
            peakHourCount = hourBuckets[i]; peakHourIdx = i;
        }
    }

    // 紧急 / 来源
    int urgentCount = 0, manualCount = 0, aiCount = 0;
    for (const auto& e : events) {
        if (e.priority == "urgent") urgentCount++;
        if (e.source == "manual") manualCount++;
        else if (e.source == "ai_parse" || e.source == "chat") aiCount++;
    }

    // 真正的日均
    double avgDaily = events.isEmpty() ? 0 : (double)events.size() / days;

    // =============================================================
    // 喂数据 → 各组件
    // =============================================================

    // [1] 概览卡片
    QVector<StatCard> cards;
    {
        StatCard c1;
        c1.icon = "📅"; c1.value = QString::number(events.size());
        c1.label = "总日程数";
        Delta d = makeDelta(events.size(), prevEvents.size());
        if (d.valid) { c1.delta = d.text; c1.deltaColor = d.color; }
        cards.append(c1);

        StatCard c2;
        c2.icon = "⏱️"; c2.value = formatMinutes(nonRestMinutes);
        c2.label = "总时长";
        Delta d2 = makeDelta(nonRestMinutes, prevNonRestMinutes);
        if (d2.valid) { c2.delta = d2.text; c2.deltaColor = d2.color; }
        cards.append(c2);

        StatCard c3;
        c3.icon = "📈"; c3.value = QString::number(avgDaily, 'f', 1);
        c3.label = "日均日程";
        cards.append(c3);

        StatCard c4;
        c4.icon = "🔴"; c4.value = QString::number(urgentCount);
        c4.label = "紧急日程";
        cards.append(c4);
    }
    m_statsCards->setCards(cards);

    // [2] 智能洞察
    QStringList insights;
    if (events.isEmpty()) {
        insights << QString("%1还没有任何日程，先去日历页添加一些试试。")
                        .arg(m_period == WeekPeriod ? "本周" : "近 30 天");
    } else {
        if (!sortedStats.isEmpty()) {
            const auto& top = sortedStats.first();
            insights << QString("时间投入最多的是「%1」，占 %2%，共 %3。")
                            .arg(top.label).arg((int)std::round(top.pct))
                            .arg(formatMinutes(top.mins));
        }
        if (busiestCount > 0) {
            QDate bd = QDate::fromString(busiestDateKey, Qt::ISODate);
            insights << QString("最忙的一天是 %1，共 %2 项日程。")
                            .arg(bd.toString("M月d日")).arg(busiestCount);
        }
        if (peakHourIdx >= 0) {
            insights << QString("日程最常安排在 %1:00 前后，可考虑将这段时间留给重要任务。")
                            .arg(peakHourIdx, 2, 10, QChar('0'));
        }
        if (prevEvents.size() > 0) {
            int diff = events.size() - prevEvents.size();
            if (std::abs(diff) >= 2) {
                QString unit = (m_period == WeekPeriod) ? "周" : "月";
                if (diff > 0)
                    insights << QString("比上一%1多了 %2 项日程，节奏更紧凑。")
                                    .arg(unit).arg(diff);
                else
                    insights << QString("比上一%1少了 %2 项，可以适当增加规划。")
                                    .arg(unit).arg(-diff);
            }
        }
        // 缺失类目提醒
        QStringList missed;
        bool hasExercise = false, hasRest = false;
        double restPct = 0, exerciseMins = 0;
        for (const auto& s : sortedStats) {
            if (s.cat == "rest") { hasRest = true; restPct = s.pct; }
            if (s.cat == "exercise") { hasExercise = true; exerciseMins = s.mins; }
        }
        if (!hasRest || restPct < 10) missed << "「休息」";
        if (!hasExercise || exerciseMins < 30) missed << "「运动」";
        if (!missed.isEmpty()) {
            insights << QString("%1没有记录%2，注意劳逸结合。")
                            .arg(m_period == WeekPeriod ? "本周" : "近 30 天")
                            .arg(missed.join("和"));
        }
        while (insights.size() > 4) insights.removeLast();
    }
    m_insights->setInsights(insights);

    // [3] 饼图 + 图例
    QVector<PieSlice> slices;
    for (const auto& s : sortedStats) {
        PieSlice ps;
        ps.label = s.label;
        ps.value = s.mins;
        ps.color = colorFromName(s.color);
        slices.append(ps);
    }
    m_pie->setSlices(slices);

    // 重建图例
    {
        QLayout* old = m_pieLegend->layout();
        if (old) {
            QLayoutItem* item;
            while ((item = old->takeAt(0)) != nullptr) {
                if (item->widget()) item->widget()->deleteLater();
                delete item;
            }
        }
        QVBoxLayout* legend = qobject_cast<QVBoxLayout*>(m_pieLegend->layout());
        if (!legend) {
            legend = new QVBoxLayout(m_pieLegend);
            legend->setContentsMargins(0, 0, 0, 0);
            legend->setSpacing(6);
        }
        for (const auto& s : sortedStats) {
            auto* row = new QWidget(m_pieLegend);
            auto* rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 0, 0, 0);
            rl->setSpacing(8);

            auto* dot = new QLabel(row);
            dot->setFixedSize(10, 10);
            QString cssColor = colorFromName(s.color).name();
            dot->setStyleSheet(QString(
                "background: %1; border-radius: 5px;").arg(cssColor));
            rl->addWidget(dot);

            auto* lbl = new QLabel(s.label, row);
            lbl->setStyleSheet("color: palette(window-text); font-size: 13px;");
            rl->addWidget(lbl);

            rl->addStretch();

            auto* val = new QLabel(
                QString("%1% (%2)").arg((int)std::round(s.pct))
                                   .arg(formatMinutes(s.mins)),
                row);
            val->setStyleSheet("font-size: 13px; font-weight: 600;");
            rl->addWidget(val);

            legend->addWidget(row);
        }
        if (sortedStats.isEmpty()) {
            auto* lbl = new QLabel("暂无数据", m_pieLegend);
            lbl->setStyleSheet("color: gray; font-size: 13px;");
            lbl->setAlignment(Qt::AlignCenter);
            legend->addWidget(lbl);
        }
    }

    // [4] Top 5 横向柱状图（按小时）
    QVector<BarItem> barData;
    for (int i = 0; i < std::min(5, (int)sortedStats.size()); ++i) {
        const auto& s = sortedStats[i];
        BarItem b;
        b.label = s.label;
        b.value = s.mins / 60.0;     // 小时
        b.suffix = "h";
        b.color = colorFromName(s.color);
        barData.append(b);
    }
    m_barChart->setData(barData);

    // [5] 每日趋势
    m_trendChart->setData(QVector<TrendPoint>(trend.cbegin(), trend.cend()));

    // [6] 节奏卡片
    if (busiestCount > 0) {
        QDate bd = QDate::fromString(busiestDateKey, Qt::ISODate);
        m_busiestCard->setValue(QString("%1 · %2 项").arg(bd.toString("M-d"))
                                                       .arg(busiestCount));
    } else {
        m_busiestCard->setValue("—");
    }
    if (peakHourIdx >= 0) {
        m_peakHourCard->setValue(
            QString("%1:00 前后").arg(peakHourIdx, 2, 10, QChar('0')));
    } else {
        m_peakHourCard->setValue("—");
    }

    // [7] 来源分布
    m_sourceWidget->setData(manualCount, aiCount);
}

} // namespace cadence
