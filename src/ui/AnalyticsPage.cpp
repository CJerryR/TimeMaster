//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

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
#include "../core/Database.h"
#include "../core/I18n.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QDate>
#include <QTimer>
#include <QSignalBlocker>

namespace timemaster {

// V3.3 had 14, V4 spec says 12; keep V3.3
static constexpr int CARD_RADIUS = 14;

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

void AnalyticsPage::buildUI() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(20, 18, 20, 18);
    outerLayout->setSpacing(14);

    // === Title bar ===
    auto *header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(8);

    m_titleIcon = new QLabel;
    m_titleIcon->setFixedSize(26, 26);
    header->addWidget(m_titleIcon);

    m_title = new QLabel;
    m_title->setObjectName("AnalyticsTitle");
    QFont titleFont;
    // V4.2 §3: page title 28px (~21pt)
    titleFont.setPointSize(21);
    titleFont.setWeight(QFont::Bold);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, -0.3);
    m_title->setFont(titleFont);
    header->addWidget(m_title);
    header->addStretch();

    // V4.3 #11 — 在刷新按钮左边加一个"已更新到 14:32"反馈标签。
    // 用户反馈：点了刷新没有视觉变化，不知道是不是真的刷新了。这条专治。
    m_updatedLabel = new QLabel;
    m_updatedLabel->setObjectName("AnalyticsUpdatedAt");
    m_updatedLabel->setMinimumHeight(28);
    header->addWidget(m_updatedLabel);

    // V4.2 #2 — bigger refresh button so the text doesn't look cramped
    m_refreshBtn = new QPushButton;
    m_refreshBtn->setObjectName("RefreshBtn");
    m_refreshBtn->setMinimumHeight(38);
    m_refreshBtn->setMinimumWidth(100);
    m_refreshBtn->setIconSize(QSize(16, 16));
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_refreshBtn, &QPushButton::clicked, this, &AnalyticsPage::refresh);
    header->addWidget(m_refreshBtn);

    m_rangeCombo = new QComboBox();
    m_rangeCombo->setFixedWidth(180);   // V4.3 — 加宽 All time 文字也放得下
    m_rangeCombo->setMinimumHeight(38);
    m_rangeCombo->setFocusPolicy(Qt::ClickFocus);
    header->addWidget(m_rangeCombo);
    outerLayout->addLayout(header);

    // === Scroll content ===
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

    // 1. Top four KPI cards
    m_statsCards = new StatsCardsWidget();
    contentLayout->addWidget(m_statsCards);

    // 2. Daily slogan + insight
    auto *motivationFrame = makeCardFrame();
    auto *motLay = new QVBoxLayout(motivationFrame);
    motLay->setContentsMargins(22, 18, 22, 18);
    m_motivationWidget = new MotivationWidget(m_db);
    motLay->addWidget(m_motivationWidget);
    contentLayout->addWidget(motivationFrame);

    // 3. Past / next week comparison
    auto *cmpFrame = makeCardFrame();
    auto *cmpLay = new QVBoxLayout(cmpFrame);
    cmpLay->setContentsMargins(20, 16, 20, 16);
    m_comparisonWidget = new ComparisonWidget(m_db);
    cmpLay->addWidget(m_comparisonWidget);
    contentLayout->addWidget(cmpFrame);

    // 4. Pie + horizontal bar
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

    // 5. Daily trend
    auto *trendFrame = makeCardFrame();
    auto *trendLay = new QVBoxLayout(trendFrame);
    trendLay->setContentsMargins(20, 16, 20, 16);
    m_trendTitle = new QLabel;
    m_trendTitle->setProperty("class", "subtitle");
    trendLay->addWidget(m_trendTitle);
    m_trendChart = new DailyTrendChart();
    trendLay->addWidget(m_trendChart, 1);
    contentLayout->addWidget(trendFrame);

    // 6. Rhythm + source distribution
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

    // 7. Insights
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

void AnalyticsPage::applyLanguage() {
    if (m_title)       m_title->setText(I18n::t("analytics.title"));
    if (m_refreshBtn) {
        m_refreshBtn->setText(QStringLiteral("  ") + I18n::t("analytics.refresh"));
        m_refreshBtn->setToolTip(I18n::t("analytics.refresh_tip"));
    }
    if (m_pieTitle)    m_pieTitle->setText(I18n::t("widget.category_share"));
    if (m_barTitle)    m_barTitle->setText(I18n::t("widget.category_time"));
    if (m_trendTitle)  m_trendTitle->setText(I18n::t("widget.daily_trend"));

    if (m_rangeCombo) {
        int idx = m_rangeCombo->currentIndex();
        QSignalBlocker blocker(m_rangeCombo);
        m_rangeCombo->clear();
        m_rangeCombo->addItem(I18n::t("analytics.range.this_week"));
        m_rangeCombo->addItem(I18n::t("analytics.range.this_month"));
        m_rangeCombo->addItem(I18n::t("analytics.range.last_7"));
        m_rangeCombo->addItem(I18n::t("analytics.range.last_30"));
        // V4.3 #11 — 新增 "全部时间 / All time"。用户反馈：点刷新还是 0 日程
        // 的根因之一是默认 Last 7 只看过去 7 天，把未来日程过滤掉了。这里把
        // All time 设为默认选项，覆盖所有 db 数据。
        m_rangeCombo->addItem(I18n::t("analytics.range.all_time"));
        // 默认改成 "All time"（index 4），用户至少能在新增日程后立即看到
        m_rangeCombo->setCurrentIndex(idx < 0 ? 4 : idx);
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
        start = QDateTime(today.addDays(-29), QTime(0, 0));
        end = QDateTime(today, QTime(23, 59, 59));
        break;
    case 4:
    default: {
        // V4.3 #11 — All time：覆盖 db 里所有事件，把未来日程也算进去。
        // 实现方式：取数据库里实际事件的最早 / 最晚时间，再用一个很宽的 fallback
        // 保证空库也不会崩。
        auto allEvts = m_db->getAllEvents();
        QDateTime minDt = QDateTime(QDate(2000, 1, 1), QTime(0, 0));
        QDateTime maxDt = QDateTime(QDate(2100, 1, 1), QTime(23, 59, 59));
        if (!allEvts.isEmpty()) {
            minDt = allEvts.first().startDate;
            maxDt = allEvts.first().endDate;
            for (const auto &e : allEvts) {
                if (e.startDate < minDt) minDt = e.startDate;
                if (e.endDate   > maxDt) maxDt = e.endDate;
            }
        }
        start = minDt;
        end   = maxDt;
        break;
    }
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

    if (m_comparisonWidget) m_comparisonWidget->refresh();
    if (m_motivationWidget) m_motivationWidget->refresh(start, end);

    // V4.3 #11 — 把"已更新到 HH:mm"写到刷新按钮旁边，给用户视觉确认
    if (m_updatedLabel) {
        QString stamp = QDateTime::currentDateTime().toString("HH:mm:ss");
        m_updatedLabel->setText(I18n::t("analytics.updated_fmt").arg(stamp));
    }

    applyTheme();
}

void AnalyticsPage::onRangeChanged() { refresh(); }

void AnalyticsPage::applyTheme() {
    auto &t = Theme::instance();
    QString brandHover = t.mode() == Theme::Light ? "#A85638" : "#D97757";
    // FIX: QString.arg() 永远替换编号最小的占位符。原来用 %6/%7 但提供了 7 个
    // .arg()，结果 %6 被填成 bgContainer（亮色下接近白）→ 白底白字。改成 %1/%2。
    setStyleSheet(t.globalStylesheet() + QString(R"(
        QPushButton#RefreshBtn {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 0 18px;
            font-weight: 600;
            font-size: 15px;
            outline: 0;
        }
        QPushButton#RefreshBtn:hover {
            background-color: %2;
        }
        QComboBox {
            font-size: 15px;
            padding: 6px 12px;
        }
    )")
    .arg(t.brand().name())   // %1 — actual brand color
    .arg(brandHover));        // %2

    // V4.2 fix: refresh按钮颜色直接写到widget自己的stylesheet上，避免被全局
    // QPushButton 样式压过去（双保险）。
    if (m_refreshBtn) {
        m_refreshBtn->setStyleSheet(QString(
            "QPushButton#RefreshBtn{background-color:%1;color:white;border:none;"
            "border-radius:10px;padding:0 18px;font-weight:600;font-size:15px;outline:0;}"
            "QPushButton#RefreshBtn:hover{background-color:%2;}")
            .arg(t.brand().name()).arg(brandHover));
    }

    // V4.3 #11 — updated 标签的样式：副色 + 小字 + 单倍间距，不抢戏
    if (m_updatedLabel) {
        m_updatedLabel->setStyleSheet(QString(
            "color:%1;background:transparent;font-size:13px;padding:0 12px 0 0;")
            .arg(t.textPlaceholder().name()));
    }

    QString cardStyle = QString(
        "QFrame#cardFrame{background:%1;border:1px solid %2;border-radius:%3px;}")
        .arg(t.cardBgRgba())
        .arg(t.strokeRgba())
        .arg(CARD_RADIUS);
    for (auto *child : findChildren<QFrame *>()) {
        if (child->objectName() == "cardFrame")
            child->setStyleSheet(cardStyle);
    }
    m_title->setStyleSheet(QString("color:%1;background:transparent;font-size:28px;font-weight:700;letter-spacing:-0.3px;").arg(t.textPrimary().name()));

    if (m_titleIcon) {
        m_titleIcon->setPixmap(IconRenderer::pixmap(IconRenderer::NavAnalytics, t.brand(), 26));
    }
    if (m_refreshBtn) {
        m_refreshBtn->setIcon(IconRenderer::icon(IconRenderer::Refresh, QColor("#FFFFFF"), 16));
    }
}

} // namespace timemaster
