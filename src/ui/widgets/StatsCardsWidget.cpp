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

    applyCardStyle(m_totalCard);
    applyCardStyle(m_countCard);
    applyCardStyle(m_avgCard);
    applyCardStyle(m_peakCard);
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
                          t.stroke().name())
                     .arg(CARD_RADIUS);
    card.frame->setStyleSheet(st);
    card.value->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPrimary().name()));
    card.frame->setMinimumWidth(130);
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
    m_totalCard.value->setText(formatMinutes(minutes));
}

void StatsCardsWidget::setCount(int count) {
    m_countCard.value->setText(QString::number(count));
}

void StatsCardsWidget::setDailyAvg(qint64 minutes) {
    m_avgCard.value->setText(formatMinutes(minutes));
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
    if (!date.isValid()) {
        m_peakCard.value->setText("—");
        m_peakCard.sub->setText("");
        return;
    }
    auto &t = Theme::instance();
    m_peakCard.value->setText(QString("%1/%2").arg(date.month()).arg(date.day()));
    m_peakCard.value->setStyleSheet(QString("color:%1;background:transparent;font-size:22px;font-weight:bold;")
                                        .arg(t.brand().name()));
    int dow = date.dayOfWeek() % 7;
    QStringList weekdays = {"日", "一", "二", "三", "四", "五", "六"};
    m_peakCard.sub->setText("周" + weekdays[dow]);
}

} // namespace timemaster
