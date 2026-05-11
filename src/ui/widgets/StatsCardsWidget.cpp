#include "StatsCardsWidget.h"
#include "../Theme.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>

namespace timemaster {

static constexpr int CARD_RADIUS = 12;

StatsCardsWidget::StatsCardsWidget(QWidget *parent) : QWidget(parent) {
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(16);

    setupCard(m_totalCard, "总时长", root);
    setupCard(m_countCard, "事件数", root);
    setupCard(m_avgCard,   "日均",   root);
    setupCard(m_peakCard,  "最忙日", root);

    // 主题变化时重新刷色
    connect(&Theme::instance(), &Theme::changed, this, &StatsCardsWidget::applyTheme);
    applyTheme();
}

void StatsCardsWidget::setupCard(Card &card, const QString &caption, QBoxLayout *row) {
    card.frame = new QFrame();
    card.frame->setObjectName("cardFrame");
    card.frame->setFrameShape(QFrame::NoFrame);
    auto *lay = new QVBoxLayout(card.frame);
    lay->setContentsMargins(20, 16, 20, 16);
    lay->setSpacing(4);

    card.caption = new QLabel(caption);
    card.caption->setProperty("class", "caption");
    lay->addWidget(card.caption);

    card.value = new QLabel("—");
    QFont nf;
    nf.setPointSize(22);
    nf.setWeight(QFont::Bold);
    card.value->setFont(nf);
    lay->addWidget(card.value);

    card.sub = new QLabel();
    card.sub->setProperty("class", "caption");
    lay->addWidget(card.sub);

    row->addWidget(card.frame, 1);
}

void StatsCardsWidget::applyCardStyle(Card &card) {
    auto &t = Theme::instance();
    QString st = QString("QFrame#cardFrame{background:%1;border:1px solid %2;border-radius:%3px;}")
                     .arg(t.bgContainer().name(),
                          t.strokeRgba())
                     .arg(CARD_RADIUS);
    card.frame->setStyleSheet(st);
    // 关键修复（item 12）：数字颜色用 textPrimary，跟随主题变成浅色
    card.value->setStyleSheet(QString("color:%1;background:transparent;")
                                  .arg(t.textPrimary().name()));
    card.caption->setStyleSheet(QString("color:%1;background:transparent;")
                                    .arg(t.textSecondary().name()));
    card.frame->setMinimumWidth(130);
}

void StatsCardsWidget::applyTheme() {
    applyCardStyle(m_totalCard);
    applyCardStyle(m_countCard);
    applyCardStyle(m_avgCard);
    applyCardStyle(m_peakCard);
    // 重新设置内容以触发依赖主题的颜色（如最忙日的 brand 色、日均的状态色）
    setTotal(m_lastTotal);
    setCount(m_lastCount);
    setDailyAvg(m_lastAvg);
    setPeakDay(m_lastPeak);
}

QString StatsCardsWidget::formatMinutes(qint64 min) const {
    if (min <= 0) return "0";
    qint64 h = min / 60;
    qint64 m = min % 60;
    if (h == 0) return QString("%1m").arg(m);
    if (m == 0) return QString("%1h").arg(h);
    return QString("%1h %2m").arg(h).arg(m);
}

void StatsCardsWidget::setTotal(qint64 minutes) {
    m_lastTotal = minutes;
    m_totalCard.value->setText(formatMinutes(minutes));
    m_totalCard.value->setStyleSheet(QString("color:%1;background:transparent;")
                                          .arg(Theme::instance().textPrimary().name()));
}

void StatsCardsWidget::setCount(int count) {
    m_lastCount = count;
    m_countCard.value->setText(QString::number(count));
    m_countCard.value->setStyleSheet(QString("color:%1;background:transparent;")
                                          .arg(Theme::instance().textPrimary().name()));
}

void StatsCardsWidget::setDailyAvg(qint64 minutes) {
    m_lastAvg = minutes;
    m_avgCard.value->setText(formatMinutes(minutes));
    m_avgCard.value->setStyleSheet(QString("color:%1;background:transparent;")
                                        .arg(Theme::instance().textPrimary().name()));
    auto &t = Theme::instance();
    if (minutes < 60) {
        m_avgCard.sub->setText("记录较少");
        m_avgCard.sub->setStyleSheet(QString("color:%1;").arg(t.textPlaceholder().name()));
    } else if (minutes > 480) {
        m_avgCard.sub->setText("高强度");
        m_avgCard.sub->setStyleSheet(QString("color:%1;").arg(t.danger().name()));
    } else {
        m_avgCard.sub->setText("适中");
        m_avgCard.sub->setStyleSheet(QString("color:%1;").arg(t.success().name()));
    }
}

void StatsCardsWidget::setPeakDay(const QDate &date) {
    m_lastPeak = date;
    if (!date.isValid()) {
        m_peakCard.value->setText("—");
        m_peakCard.value->setStyleSheet(QString("color:%1;background:transparent;")
                                              .arg(Theme::instance().textPrimary().name()));
        m_peakCard.sub->setText("");
        return;
    }
    auto &t = Theme::instance();
    m_peakCard.value->setText(QString("%1/%2").arg(date.month()).arg(date.day()));
    // 最忙日用 brand 色（橙）—— 两种主题下都对比鲜明
    m_peakCard.value->setStyleSheet(QString("color:%1;background:transparent;font-weight:bold;")
                                        .arg(t.brand().name()));
    int dow = date.dayOfWeek() % 7;
    QStringList weekdays = {"日", "一", "二", "三", "四", "五", "六"};
    m_peakCard.sub->setText("周" + weekdays[dow]);
    m_peakCard.sub->setStyleSheet(QString("color:%1;").arg(t.textSecondary().name()));
}

} // namespace timemaster
