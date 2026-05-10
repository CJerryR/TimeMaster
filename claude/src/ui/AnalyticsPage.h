// ============================================================
// AnalyticsPage.h
// 完整版分析页 — 对齐网页版所有模块：
//   1. 4 张概览卡片 (StatsCardsWidget) — 总日程/总时长/日均/紧急 + 环比
//   2. 智能洞察 (InsightsWidget)
//   3. 时间分配饼图 + 图例 (CategoryPieChart, 已有，沿用)
//   4. 分类耗时 Top 5 横向柱状图 (HorizontalBarChart)
//   5. 每日趋势 mini 柱状图 (DailyTrendChart)
//   6. 节奏卡片：最忙的一天 / 高峰时段 (RhythmCardWidget × 2)
//   7. 录入方式分布 (SourceDistributionWidget)
// ============================================================
#pragma once
#include <QWidget>
#include <QDate>
#include "../core/Types.h"

class QPushButton;
class QButtonGroup;
class QLabel;
class QScrollArea;

namespace cadence {

class Database;
class CategoryPieChart;
class StatsCardsWidget;
class InsightsWidget;
class HorizontalBarChart;
class DailyTrendChart;
class RhythmCardWidget;
class SourceDistributionWidget;

class AnalyticsPage : public QWidget {
    Q_OBJECT
public:
    explicit AnalyticsPage(Database* db, QWidget* parent = nullptr);

public slots:
    void refresh();

private slots:
    void onPeriodChanged(int periodId); // 0=week 1=month

private:
    enum Period { WeekPeriod = 0, MonthPeriod = 1 };

    void buildUi();
    void rebuildStats();

    // 工具
    static double clampedMinutes(const CalendarEvent& e,
                                  const QDateTime& periodStart,
                                  const QDateTime& periodEnd,
                                  const QDateTime& now);
    static QString formatMinutes(double m);
    struct Delta { QString text; QString color; bool valid{false}; };
    static Delta makeDelta(double curr, double prev);

    Database* m_db;
    Period m_period{WeekPeriod};

    // 控件
    QButtonGroup* m_periodGroup{nullptr};
    QPushButton* m_refreshBtn{nullptr};
    QScrollArea* m_scroll{nullptr};

    StatsCardsWidget* m_statsCards{nullptr};
    InsightsWidget* m_insights{nullptr};
    CategoryPieChart* m_pie{nullptr};
    QWidget* m_pieLegend{nullptr};
    HorizontalBarChart* m_barChart{nullptr};
    DailyTrendChart* m_trendChart{nullptr};
    RhythmCardWidget* m_busiestCard{nullptr};
    RhythmCardWidget* m_peakHourCard{nullptr};
    SourceDistributionWidget* m_sourceWidget{nullptr};
};

} // namespace cadence
