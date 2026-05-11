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

private:
    void buildUI();
    QFrame *makeCardFrame();

    Database *m_db;

    QLabel *m_title;
    QLabel *m_titleIcon = nullptr;
    QComboBox *m_rangeCombo;
    QPushButton *m_refreshBtn = nullptr;

    // V4 § 6.3: section headers
    QLabel *m_secOverview  = nullptr;
    QLabel *m_secStructure = nullptr;
    QLabel *m_secInsights  = nullptr;
    QList<QFrame*> m_sectionAccentBars;

    StatsCardsWidget *m_statsCards;
    CategoryPieChart *m_pieChart;
    HorizontalBarChart *m_barChart;
    DailyTrendChart *m_trendChart;
    RhythmCardWidget *m_rhythmWidget;
    SourceDistributionWidget *m_sourceWidget;
    InsightsWidget *m_insightsWidget;
    ComparisonWidget *m_comparisonWidget = nullptr;
    MotivationWidget *m_motivationWidget = nullptr;

    QScrollArea *m_scrollArea;

    // V4 § 5.2: empty state shown when 7d events == 0
    QWidget    *m_scrollContent = nullptr;
    class EmptyState *m_emptyState = nullptr;
    QWidget    *m_contentHost = nullptr;

    // Subtitle labels (for language refresh)
    QLabel *m_pieTitle = nullptr;
    QLabel *m_barTitle = nullptr;
    QLabel *m_trendTitle = nullptr;

private slots:
    void applyLanguage();
};

} // namespace timemaster
