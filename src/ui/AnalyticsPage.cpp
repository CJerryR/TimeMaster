#include "AnalyticsPage.h"
#include "CategoryPieChart.h"
#include "Theme.h"
#include "widgets/StatsCardsWidget.h"
#include "widgets/HorizontalBarChart.h"
#include "widgets/DailyTrendChart.h"
#include "widgets/RhythmCardWidget.h"
#include "widgets/SourceDistributionWidget.h"
#include "widgets/InsightsWidget.h"
#include "../core/Database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QDate>
#include <QTimer>

namespace timeplan {

static constexpr int CARD_RADIUS = 12;

AnalyticsPage::AnalyticsPage(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    buildUI();
    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, &AnalyticsPage::applyTheme);
}

void AnalyticsPage::buildUI() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    m_title = new QLabel("📊 统计分析");
    m_title->setProperty("class", "pagetitle");
    header->addWidget(m_title);
    header->addStretch();

    m_rangeCombo = new QComboBox();
    m_rangeCombo->addItems({"本周", "本月", "近 7 天", "近 30 天"});
    m_rangeCombo->setFixedWidth(120);
    header->addWidget(m_rangeCombo);
    outerLayout->addLayout(header);
    outerLayout->addSpacing(8);

    // ---- Scroll Area ----
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *content = new QWidget();
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 32);
    contentLayout->setSpacing(20);

    // ---- Row 0 · 统计卡片 ----
    m_statsCards = new StatsCardsWidget();
    contentLayout->addWidget(m_statsCards);

    // ---- Card 1 · 类别饼图 & 条形图并排 ----
    auto *chartRow = new QHBoxLayout();
    chartRow->setSpacing(16);

    m_pieChart = new CategoryPieChart();
    m_barChart = new HorizontalBarChart();
    chartRow->addWidget(m_pieChart, 4);
    chartRow->addWidget(m_barChart, 6);

    contentLayout->addLayout(chartRow);

    // ---- Card 2 · 每日趋势 ----
    auto *trendFrame = makeCardFrame();
    auto *trendLay = new QVBoxLayout(trendFrame);
    trendLay->setContentsMargins(20, 16, 20, 16);
    auto *trendTitle = new QLabel("每日趋势");
    trendTitle->setProperty("class", "subtitle");
    trendLay->addWidget(trendTitle);
    m_trendChart = new DailyTrendChart();
    trendLay->addWidget(m_trendChart, 1);
    contentLayout->addWidget(trendFrame);

    // ---- Row 3 · 节奏 + 来源分布 ----
    auto *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(16);

    auto *rhythmFrame = makeCardFrame();
    auto *rhythmLay = new QVBoxLayout(rhythmFrame);
    rhythmLay->setContentsMargins(20, 16, 20, 16);
    m_rhythmWidget = new RhythmCardWidget();
    rhythmLay->addWidget(m_rhythmWidget, 1);
    bottomRow->addWidget(rhythmFrame, 5);

    auto *sourceFrame = makeCardFrame();
    auto *sourceLay = new QVBoxLayout(sourceFrame);
    sourceLay->setContentsMargins(20, 16, 20, 16);
    m_sourceWidget = new SourceDistributionWidget();
    sourceLay->addWidget(m_sourceWidget, 1);
    bottomRow->addWidget(sourceFrame, 5);

    contentLayout->addLayout(bottomRow);

    // ---- Card 4 · Insights ----
    auto *insightsFrame = makeCardFrame();
    auto *insightsLay = new QVBoxLayout(insightsFrame);
    insightsLay->setContentsMargins(20, 16, 20, 16);
    m_insightsWidget = new InsightsWidget(m_db);
    insightsLay->addWidget(m_insightsWidget, 1);
    contentLayout->addWidget(insightsFrame);

    scroll->setWidget(content);
    outerLayout->addWidget(scroll, 1);
    m_scrollArea = scroll;

    connect(m_rangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnalyticsPage::onRangeChanged);
}

QFrame *AnalyticsPage::makeCardFrame() {
    auto *f = new QFrame();
    f->setObjectName("cardFrame");
    f->setFrameShape(QFrame::NoFrame);
    return f;
}

void AnalyticsPage::refresh() {
    QDateTime start, end;
    int idx = m_rangeCombo->currentIndex();
    QDate today = QDate::currentDate();

    switch (idx) {
    case 0: // 本周
        start = QDateTime(today.addDays(-(today.dayOfWeek() % 7 + 6) % 7), QTime(0, 0));
        end = QDateTime(today, QTime(23, 59, 59));
        break;
    case 1: // 本月
        start = QDateTime(QDate(today.year(), today.month(), 1), QTime(0, 0));
        end = QDateTime(today, QTime(23, 59, 59));
        break;
    case 2: // 近 7 天
        start = QDateTime(today.addDays(-6), QTime(0, 0));
        end = QDateTime(today, QTime(23, 59, 59));
        break;
    case 3: // 近 30 天
    default:
        start = QDateTime(today.addDays(-29), QTime(0, 0));
        end = QDateTime(today, QTime(23, 59, 59));
        break;
    }

    auto stats = m_db->getCategoryStats(start, end);
    auto daily = m_db->getDailySummaries(start, end);
    auto hourly = m_db->getHourlyDistribution(start, end);
    int manualC = m_db->eventCountBySource(EventSource::Manual, start, end);
    int aiC = m_db->eventCountBySource(EventSource::AiParse, start, end);
    int importC = m_db->eventCountBySource(EventSource::Chat, start, end);

    // 填充卡片
    qint64 totalMin = 0, totalCnt = 0;
    for (auto &s : stats) { totalMin += s.totalMinutes; totalCnt += s.count; }
    int days = qMax(1, int(start.daysTo(end)));
    m_statsCards->setTotal(totalMin);
    m_statsCards->setCount(int(totalCnt));
    m_statsCards->setDailyAvg(totalMin / days);

    QDate peakDay;
    qint64 peakMin = 0;
    for (const auto &d : daily) {
        if (d.totalMinutes > peakMin) { peakMin = d.totalMinutes; peakDay = d.date; }
    }
    m_statsCards->setPeakDay(peakDay);

    // 图表
    m_pieChart->setStats(stats);
    m_barChart->setData(stats);
    m_trendChart->setData(daily);
    m_rhythmWidget->setHourlyData(hourly);
    m_sourceWidget->setSources(manualC, aiC, importC);
    m_insightsWidget->refresh(start, end);

    // 卡片背景
    applyTheme();
}

void AnalyticsPage::onRangeChanged() { refresh(); }

void AnalyticsPage::applyTheme() {
    auto &t = Theme::instance();
    QString cardStyle = QString("QFrame#cardFrame{background:%1;border:1px solid %2;border-radius:%3px;}")
                            .arg(t.bgContainer().name(), t.stroke().name())
                            .arg(CARD_RADIUS);
    for (auto *child : findChildren<QFrame *>()) {
        if (child->objectName() == "cardFrame")
            child->setStyleSheet(cardStyle);
    }
}

} // namespace timeplan
