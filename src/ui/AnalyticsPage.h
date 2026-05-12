#pragma once

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QScrollArea>
#include <QPushButton>

#include "../core/Types.h"

namespace timemaster {

class Database;
class CategoryPieChart;
class HorizontalBarChart;
class DailyTrendChart;
class StatsCardsWidget;
class SourceDistributionWidget;
class InsightsWidget;
class RhythmCardWidget;
class ComparisonWidget;
class MotivationWidget;

class AnalyticsPage : public QWidget {
    Q_OBJECT
public:
    explicit AnalyticsPage(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onRangeChanged();
    void applyTheme();
    void applyLanguage();

private:
    void buildUI();
    QFrame *makeCardFrame();

    Database *m_db;

    QLabel *m_title;
    QLabel *m_titleIcon = nullptr;
    QComboBox *m_rangeCombo;
    QPushButton *m_refreshBtn = nullptr;
    // V4.3 #11 — "数据已更新到 14:32" 反馈标签，放在刷新按钮旁
    QLabel *m_updatedLabel = nullptr;

    StatsCardsWidget *m_statsCards;
    CategoryPieChart *m_pieChart;
    HorizontalBarChart *m_barChart;
    DailyTrendChart *m_trendChart;
    RhythmCardWidget *m_rhythmWidget;
    SourceDistributionWidget *m_sourceWidget;
    InsightsWidget *m_insightsWidget;
    ComparisonWidget *m_comparisonWidget = nullptr;
    MotivationWidget *m_motivationWidget = nullptr;

    // Subtitle labels (for i18n refresh)
    QLabel *m_pieTitle = nullptr;
    QLabel *m_barTitle = nullptr;
    QLabel *m_trendTitle = nullptr;

    QScrollArea *m_scrollArea;
};

} // namespace timemaster
