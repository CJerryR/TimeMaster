#pragma once

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QScrollArea>

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

class AnalyticsPage : public QWidget {
    Q_OBJECT
public:
    explicit AnalyticsPage(Database *db, QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onRangeChanged();

private:
    void buildUI();
    void applyTheme();
    QFrame *makeCardFrame();

    Database *m_db;

    QLabel *m_title;
    QComboBox *m_rangeCombo;

    StatsCardsWidget *m_statsCards;
    CategoryPieChart *m_pieChart;
    HorizontalBarChart *m_barChart;
    DailyTrendChart *m_trendChart;
    RhythmCardWidget *m_rhythmWidget;
    SourceDistributionWidget *m_sourceWidget;
    InsightsWidget *m_insightsWidget;

    QScrollArea *m_scrollArea;
};

} // namespace timemaster
