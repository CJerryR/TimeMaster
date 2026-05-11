#include "AnalyticsPage.h"
#include "CategoryPieChart.h"
#include "Theme.h"
#include "IconRenderer.h"
#include "widgets/StatsCardsWidget.h"
#include "widgets/HorizontalBarChart.h"
#include "widgets/DailyTrendChart.h"
#include "widgets/RhythmCardWidget.h"
#include "widgets/SourceDistributionWidget.h"
#include "widgets/InsightsWidget.h"
#include "widgets/ComparisonWidget.h"
#include "widgets/MotivationWidget.h"
#include "widgets/EmptyState.h"
#include "../core/Database.h"
#include "../core/I18n.h"

#include <QStackedLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QDate>
#include <QSignalBlocker>

namespace timemaster {

// V4 § 6.5: cards 12px
static constexpr int CARD_RADIUS = 12;

AnalyticsPage::AnalyticsPage(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    buildUI();
    applyLanguage();
    applyTheme();
    connect(&Theme::instance(), &Theme::changed,        this, &AnalyticsPage::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, &AnalyticsPage::applyLanguage);
    connect(m_db, &Database::eventsChanged, this, &AnalyticsPage::refresh);
}

static QLabel *makeSectionHeader(QList<QFrame*> *accentBars) {
    auto *row = new QLabel;
    row->setObjectName("AnalyticsSection");
    row->setContentsMargins(0, 12, 0, 4);
    row->setProperty("class", "section");
    Q_UNUSED(accentBars);
    return row;
}

void AnalyticsPage::buildUI() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(24, 20, 24, 20);
    outerLayout->setSpacing(14);

    // === Top bar ===
    auto *header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(8);

    m_titleIcon = new QLabel;
    m_titleIcon->setFixedSize(24, 24);
    header->addWidget(m_titleIcon);

    m_title = new QLabel;
    m_title->setObjectName("AnalyticsTitle");
    QFont titleFont;
    titleFont.setPointSize(17);
    titleFont.setWeight(QFont::DemiBold);
    m_title->setFont(titleFont);
    header->addWidget(m_title);
    header->addStretch();

    m_refreshBtn = new QPushButton;
    m_refreshBtn->setObjectName("RefreshBtn");
    m_refreshBtn->setMinimumHeight(32);
    m_refreshBtn->setMinimumWidth(78);
    m_refreshBtn->setIconSize(QSize(16, 16));
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(m_refreshBtn, &QPushButton::clicked, this, &AnalyticsPage::refresh);
    header->addWidget(m_refreshBtn);

    m_rangeCombo = new QComboBox();
    m_rangeCombo->setFixedWidth(140);
    m_rangeCombo->setMinimumHeight(32);
    header->addWidget(m_rangeCombo);
    outerLayout->addLayout(header);

    // === Scrollable content host with EmptyState overlay (V4 § 5.2) ===
    m_contentHost = new QWidget;
    auto *overlay = new QStackedLayout(m_contentHost);
    overlay->setContentsMargins(0, 0, 0, 0);
    overlay->setStackingMode(QStackedLayout::StackAll);

    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{background:transparent;}");
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("QWidget{background:transparent;}");
    auto *contentLayout = new QVBoxLayout(m_scrollContent);
    contentLayout->setContentsMargins(0, 0, 0, 24);
    contentLayout->setSpacing(14);

    // ---- Section 1: Overview ----
    m_secOverview = makeSectionHeader(&m_sectionAccentBars);
    contentLayout->addWidget(m_secOverview);

    m_statsCards = new StatsCardsWidget();
    contentLayout->addWidget(m_statsCards);

    auto *motivationFrame = makeCardFrame();
    auto *motLay = new QVBoxLayout(motivationFrame);
    motLay->setContentsMargins(22, 18, 22, 18);
    m_motivationWidget = new MotivationWidget(m_db);
    motLay->addWidget(m_motivationWidget);
    contentLayout->addWidget(motivationFrame);

    auto *cmpFrame = makeCardFrame();
    auto *cmpLay = new QVBoxLayout(cmpFrame);
    cmpLay->setContentsMargins(20, 16, 20, 16);
    m_comparisonWidget = new ComparisonWidget(m_db);
    cmpLay->addWidget(m_comparisonWidget);
    contentLayout->addWidget(cmpFrame);

    // ---- Section 2: Time structure ----
    m_secStructure = makeSectionHeader(&m_sectionAccentBars);
    contentLayout->addWidget(m_secStructure);

    auto *chartRow = new QHBoxLayout();
    chartRow->setSpacing(14);

    auto *pieFrame = makeCardFrame();
    auto *pieLay = new QVBoxLayout(pieFrame);
    pieLay->setContentsMargins(20, 16, 20, 16);
    m_pieTitle = new QLabel;
    m_pieTitle->setProperty("class", "subtitle");
    pieLay->addWidget(m_pieTitle);
    m_pieChart = new CategoryPieChart();
    pieLay->addWidget(m_pieChart, 1);
    chartRow->addWidget(pieFrame, 4);

    auto *barFrame = makeCardFrame();
    auto *barLay = new QVBoxLayout(barFrame);
    barLay->setContentsMargins(20, 16, 20, 16);
    m_barTitle = new QLabel;
    m_barTitle->setProperty("class", "subtitle");
    barLay->addWidget(m_barTitle);
    m_barChart = new HorizontalBarChart();
    barLay->addWidget(m_barChart, 1);
    chartRow->addWidget(barFrame, 6);

    contentLayout->addLayout(chartRow);

    auto *sourceFrame = makeCardFrame();
    auto *sourceLay = new QVBoxLayout(sourceFrame);
    sourceLay->setContentsMargins(20, 16, 20, 16);
    m_sourceWidget = new SourceDistributionWidget();
    sourceLay->addWidget(m_sourceWidget, 1);
    contentLayout->addWidget(sourceFrame);

    // ---- Section 3: Behavioural insights ----
    m_secInsights = makeSectionHeader(&m_sectionAccentBars);
    contentLayout->addWidget(m_secInsights);

    auto *trendFrame = makeCardFrame();
    auto *trendLay = new QVBoxLayout(trendFrame);
    trendLay->setContentsMargins(20, 16, 20, 16);
    m_trendTitle = new QLabel;
    m_trendTitle->setProperty("class", "subtitle");
    trendLay->addWidget(m_trendTitle);
    m_trendChart = new DailyTrendChart();
    trendLay->addWidget(m_trendChart, 1);
    contentLayout->addWidget(trendFrame);

    auto *rhythmFrame = makeCardFrame();
    auto *rhythmLay = new QVBoxLayout(rhythmFrame);
    rhythmLay->setContentsMargins(20, 16, 20, 16);
    m_rhythmWidget = new RhythmCardWidget();
    rhythmLay->addWidget(m_rhythmWidget, 1);
    contentLayout->addWidget(rhythmFrame);

    auto *insightsFrame = makeCardFrame();
    auto *insightsLay = new QVBoxLayout(insightsFrame);
    insightsLay->setContentsMargins(20, 16, 20, 16);
    m_insightsWidget = new InsightsWidget(m_db);
    insightsLay->addWidget(m_insightsWidget, 1);
    contentLayout->addWidget(insightsFrame);

    scroll->setWidget(m_scrollContent);
    overlay->addWidget(scroll);

    m_emptyState = new EmptyState;
    overlay->addWidget(m_emptyState);

    outerLayout->addWidget(m_contentHost, 1);
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

void AnalyticsPage::applyLanguage() {
    if (m_title)        m_title->setText(I18n::t("analytics.title"));
    if (m_refreshBtn)   m_refreshBtn->setText(I18n::t("analytics.refresh"));
    if (m_refreshBtn)   m_refreshBtn->setToolTip(I18n::t("analytics.refresh_tip"));

    if (m_rangeCombo) {
        int idx = m_rangeCombo->currentIndex();
        QSignalBlocker blocker(m_rangeCombo);
        m_rangeCombo->clear();
        m_rangeCombo->addItem(I18n::t("analytics.range.this_week"));
        m_rangeCombo->addItem(I18n::t("analytics.range.this_month"));
        m_rangeCombo->addItem(I18n::t("analytics.range.last_7"));
        m_rangeCombo->addItem(I18n::t("analytics.range.last_30"));
        m_rangeCombo->setCurrentIndex(idx < 0 ? 2 : idx);
    }

    if (m_secOverview)  m_secOverview->setText(I18n::t("analytics.section.overview"));
    if (m_secStructure) m_secStructure->setText(I18n::t("analytics.section.structure"));
    if (m_secInsights)  m_secInsights->setText(I18n::t("analytics.section.insights"));

    if (m_pieTitle)   m_pieTitle->setText(I18n::t("widget.category_share"));
    if (m_barTitle)   m_barTitle->setText(I18n::t("widget.category_time"));
    if (m_trendTitle) m_trendTitle->setText(I18n::t("widget.daily_trend"));

    if (m_emptyState) {
        m_emptyState->setTitle(I18n::t("empty.analytics.title"));
        m_emptyState->setSubtitle(I18n::t("empty.analytics.subtitle"));
    }
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

    auto stats   = m_db->getCategoryStats(start, end);
    auto daily   = m_db->getDailySummaries(start, end);
    auto hourly  = m_db->getHourlyDistribution(start, end);
    int manualC  = m_db->eventCountBySource(EventSource::Manual,  start, end);
    int aiC      = m_db->eventCountBySource(EventSource::AiParse, start, end);
    int chatC    = m_db->eventCountBySource(EventSource::Chat,    start, end);

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

    if (m_comparisonWidget) m_comparisonWidget->refresh();
    if (m_motivationWidget) m_motivationWidget->refresh(start, end);

    // V4 § 5.2: empty state — use the 7-day window regardless of selected range
    if (m_emptyState && m_scrollContent) {
        QDateTime sevenStart(today.addDays(-6), QTime(0, 0));
        QDateTime sevenEnd(today, QTime(23, 59, 59));
        int recent = m_db->getEventsByRange(sevenStart, sevenEnd).size();
        bool emptyish = (recent == 0);
        if (emptyish) {
            int totalAll = m_db->getAllEvents().size();
            m_emptyState->setTitle(I18n::t("empty.analytics.title"));
            m_emptyState->setSubtitle(I18n::t("empty.analytics.subtitle"));
            m_emptyState->setProgress(I18n::t("empty.analytics.progress_fmt").arg(qMin(totalAll, 3)));
            m_emptyState->clearActions();
            m_emptyState->show();
            m_emptyState->raise();
            m_scrollContent->setVisible(false);
        } else {
            m_emptyState->hide();
            m_scrollContent->setVisible(true);
        }
    }

    applyTheme();
}

void AnalyticsPage::onRangeChanged() { refresh(); }

void AnalyticsPage::applyTheme() {
    auto &t = Theme::instance();
    setStyleSheet(t.globalStylesheet() + QString(R"(
        QPushButton#RefreshBtn {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            color: %3;
            font-weight: 500;
            padding: 0 14px;
        }
        QPushButton#RefreshBtn:hover {
            background-color: %4;
            color: %5;
        }
        QLabel#AnalyticsSection[class="section"] {
            color: %5;
            border-left: 3px solid %6;
            padding-left: 10px;
            margin-top: 18px;
            margin-bottom: 6px;
            font-size: 15px;
            font-weight: 600;
            background: transparent;
        }
    )")
    .arg(t.bgContainer().name())
    .arg(t.strokeRgba())
    .arg(t.textSecondary().name())
    .arg(t.cardBgHoverRgba())
    .arg(t.textPrimary().name())
    .arg(t.brand().name()));

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

    if (m_titleIcon) {
        m_titleIcon->setPixmap(IconRenderer::pixmap(IconRenderer::NavAnalytics, t.brand(), 24));
    }
    if (m_refreshBtn) {
        m_refreshBtn->setIcon(IconRenderer::icon(IconRenderer::Refresh, t.textSecondary(), 16));
    }
}

} // namespace timemaster
