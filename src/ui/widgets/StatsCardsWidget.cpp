#include "StatsCardsWidget.h"
#include "../Theme.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLocale>

namespace timemaster {

static constexpr int CARD_RADIUS = 12;

StatsCardsWidget::StatsCardsWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);

    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(12);

    m_totalCard = makeCard("⏱", "总时长", "#EF4444");
    m_countCard = makeCard("📅", "事件数", "#6366F1");
    m_avgCard   = makeCard("📈", "日均", "#10B981");
    m_peakCard  = makeCard("🔥", "高峰日", "#F59E0B");

    row->addWidget(m_totalCard.frame, 1);
    row->addWidget(m_countCard.frame, 1);
    row->addWidget(m_avgCard.frame,   1);
    row->addWidget(m_peakCard.frame,  1);

    setTotal(0);
    setCount(0);
    setDailyAvg(0);
    setPeakDay(QDate());

    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, &StatsCardsWidget::applyTheme);
}

StatsCardsWidget::Card StatsCardsWidget::makeCard(const QString &icon, const QString &label, const QString &accent) {
    Card c;
    c.frame = new QFrame();
    c.frame->setObjectName("statCard");
    c.frame->setFrameShape(QFrame::NoFrame);
    c.frame->setMinimumHeight(96);

    auto *lay = new QVBoxLayout(c.frame);
    lay->setContentsMargins(18, 14, 18, 14);
    lay->setSpacing(4);

    auto *top = new QHBoxLayout();
    top->setContentsMargins(0, 0, 0, 0);
    top->setSpacing(8);

    c.icon = new QLabel(icon);
    QFont iconF;
    iconF.setPointSize(16);
    c.icon->setFont(iconF);
    c.icon->setStyleSheet(QString("color:%1;background:transparent;").arg(accent));
    top->addWidget(c.icon);

    c.label = new QLabel(label);
    c.label->setProperty("class", "muted");
    QFont labelF;
    labelF.setPointSize(10);
    c.label->setFont(labelF);
    top->addWidget(c.label);
    top->addStretch();

    lay->addLayout(top);

    c.value = new QLabel("—");
    c.value->setObjectName("statValue");
    QFont vF;
    vF.setPointSize(20);
    vF.setWeight(QFont::Bold);
    c.value->setFont(vF);
    lay->addWidget(c.value);

    c.sub = new QLabel("");
    c.sub->setProperty("class", "muted");
    QFont sF;
    sF.setPointSize(9);
    c.sub->setFont(sF);
    lay->addWidget(c.sub);

    return c;
}

QString StatsCardsWidget::formatMinutes(qint64 m) const {
    if (m <= 0) return "0 分";
    qint64 h = m / 60;
    qint64 mm = m % 60;
    if (h <= 0) return QString("%1 分").arg(mm);
    if (mm == 0) return QString("%1 小时").arg(h);
    return QString("%1 小时 %2 分").arg(h).arg(mm);
}

void StatsCardsWidget::setTotal(qint64 minutes) {
    m_totalCard.value->setText(formatMinutes(minutes));
    m_totalCard.sub->setText(minutes > 0 ? QString("约 %1 小时").arg(QString::number(minutes / 60.0, 'f', 1))
                                         : "暂无数据");
}

void StatsCardsWidget::setCount(int n) {
    m_countCard.value->setText(QString::number(n));
    m_countCard.sub->setText(n > 0 ? "已完成 / 计划中" : "暂无数据");
}

void StatsCardsWidget::setDailyAvg(qint64 minutes) {
    m_avgCard.value->setText(formatMinutes(minutes));
    m_avgCard.sub->setText(minutes > 0 ? "每天平均花费" : "暂无数据");
}

void StatsCardsWidget::setPeakDay(const QDate &d) {
    if (!d.isValid()) {
        m_peakCard.value->setText("—");
        m_peakCard.sub->setText("暂无数据");
    } else {
        m_peakCard.value->setText(d.toString("MM/dd"));
        const QStringList wk = {"周一","周二","周三","周四","周五","周六","周日"};
        int idx = d.dayOfWeek() - 1;
        if (idx < 0 || idx >= wk.size()) idx = 0;
        m_peakCard.sub->setText(wk.at(idx) + " · 时长最长");
    }
}

void StatsCardsWidget::applyTheme() {
    auto &t = Theme::instance();
    QString cardStyle = QString(
        "QFrame#statCard{background:%1;border:1px solid %2;border-radius:%3px;}"
        "QFrame#statCard:hover{background:%4;}"
        "QLabel{background:transparent;}"
        "QLabel[class=\"muted\"]{color:%5;}"
        "QLabel#statValue{color:%6;}"
    )
    .arg(t.cardBgRgba())
    .arg(t.strokeRgba())
    .arg(CARD_RADIUS)
    .arg(t.cardBgHoverRgba())
    .arg(t.textSecondary().name())
    .arg(t.textPrimary().name());

    for (auto *card : {m_totalCard.frame, m_countCard.frame, m_avgCard.frame, m_peakCard.frame}) {
        if (card) card->setStyleSheet(cardStyle);
    }
}

} // namespace timemaster
