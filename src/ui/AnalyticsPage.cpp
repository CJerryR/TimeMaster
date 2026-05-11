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

namespace timemaster {

static constexpr int CARD_RADIUS = 14;

AnalyticsPage::AnalyticsPage(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    buildUI();
    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, &AnalyticsPage::applyTheme);
}

void AnalyticsPage::buildUI() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(20, 18, 20, 18);
    outerLayout->setSpacing(14);

    auto *header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    m_title = new QLabel("📊  统计分析");
    m_title->setObjectName("AnalyticsTitle");
    QFont titleFont;
    titleFont.setPointSize(17);
    titleFont.setWeight(QFont::Bold);
    m_title->setFont(titleFont);
    header->addWidget(m_title);
    header->addStretch();

    m_rangeCombo = new QComboBox();
    m_rangeCombo->addItems({"本周", "本月", "近 7 天", "近 30 天"});
    m_rangeCombo->setFixedWidth(130);
    m_rangeCombo->setMinimumHeight(34);
    header->addWidget(m_rangeCombo);
    outerLayout->addLayout(header);

    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{background:transparent;}");
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *content = new QWidget();
    content->setStyleSheet("QWidget{background:transparent;}");
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 24);
    contentLayout->setSpacing(16);

    m_statsCards = new StatsCardsWidget();
    contentLayout->addWidget(m_statsCards);

    auto *chartRow = new QHBoxLayout();
    chartRow->setSpacing(14);

    auto *pieFrame = makeCardFrame();
    auto *pieLay = new QVBoxLayout(pieFrame);
    pieLay->setContentsMargins(20, 16, 20, 16);
    auto *pieTitle = new QLabel("类别占比");
    pieTitle->setProperty("class", "subtitle");
    pieLay->addWidget(pieTitle);
    m_pieChart = new CategoryPieChart();
    pieLay->addWidget(m_pieChart, 1);
    chartRow->addWidget(pieFrame, 4);

    auto *barFrame = makeCardFrame();
    auto *barLay = new QVBoxLayout(barFrame);
    barLay->setContentsMargins(20, 16, 20, 16);
    auto *barTitle = new QLabel("类别时长");
    barTitle->setProperty("class", "subtitle");
    barLay->addWidget(barTitle);
    m_barChart = new HorizontalBarChart();
    barLay->addWidget(m_barChart, 1);
    chartRow->addWidget(barFrame, 6);

    contentLayout->addLayout(chartRow);

    auto *trendFrame = makeCardFrame();
    auto *trendLay = new QVBoxLayout(trendFrame);
    trendLay->setContentsMargins(20, 16, 20, 16);
    auto *trendTitle = new QLabel("每日趋势");
    trendTitle->setProperty("class", "subtitle");
    trendLay->addWidget(trendTitle);
    m_trendChart = new DailyTrendChart();
    trendLay->addWidget(m_trendChart, 1);
    contentLayout->addWidget(trendFrame);

    auto *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(14);

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
    case 0:
        start = QDateTime(today.addDays(-(today.dayOfWeek() % 7 + 6) % 7), QTime(0, 0));
        end = QDateTime(today, QTime(23, 59, 59));
        break;
    case 1:
        start = QDateTime(QDate(today.year(), today.month(), 1), QTime(0, 0));
        end = QDateTime(today, QTime(23, 59, 59));
        break;
    case 2:
        start = QDateTime(today.addDays(-6), QTime(0, 0));
        end = QDateTime(today, QTime(23, 59, 59));
        break;
    case 3:
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
    int chatC = m_db->eventCountBySource(EventSource::Chat, start, end);

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

    m_pieChart->setStats(stats);
    m_barChart->setData(stats);
    m_trendChart->setData(daily);
    m_rhythmWidget->setHourlyData(hourly);
    m_sourceWidget->setSources(manualC, aiC, chatC);
    m_insightsWidget->refresh(start, end);

    applyTheme();
}

void AnalyticsPage::onRangeChanged() { refresh(); }

void AnalyticsPage::applyTheme() {
    auto &t = Theme::instance();
    setStyleSheet(t.globalStylesheet());

    QString cardStyle = QString(
        "QFrame#cardFrame{background:%1;border:1px solid %2;border-radius:%3px;}")
        .arg(t.cardBgRgba())
        .arg(t.strokeRgba())
        .arg(CARD_RADIUS);
    for (auto *child : findChildren<QFrame *>()) {
        if (child->objectName() == "cardFrame")
            child->setStyleSheet(cardStyle);
    }
    m_title->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPrimary().name()));
}

} // namespace timemaster
