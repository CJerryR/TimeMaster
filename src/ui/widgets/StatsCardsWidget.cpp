//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "StatsCardsWidget.h"
#include "ShadowEffect.h"
#include "../Theme.h"
#include "../FontLoader.h"
#include "../../core/I18n.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QLocale>

namespace timemaster {

// V4 § 6.5: cards 12px
static constexpr int CARD_RADIUS = 12;

StatsCardsWidget::StatsCardsWidget(QWidget *parent) : QWidget(parent) {
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(14);

    setupCard(m_totalCard, I18n::t("kpi.total"),    root);
    setupCard(m_countCard, I18n::t("kpi.events"),   root);
    setupCard(m_avgCard,   I18n::t("kpi.daily_avg"),root);
    setupCard(m_peakCard,  I18n::t("kpi.peak_day"), root);

    connect(&Theme::instance(), &Theme::changed, this, &StatsCardsWidget::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, [this]{
        // Re-set captions on language change
        m_totalCard.caption->setText(I18n::t("kpi.total"));
        m_countCard.caption->setText(I18n::t("kpi.events"));
        m_avgCard.caption->setText(I18n::t("kpi.daily_avg"));
        m_peakCard.caption->setText(I18n::t("kpi.peak_day"));
        applyTheme();
    });
    applyTheme();
}

void StatsCardsWidget::setupCard(Card &card, const QString &caption, QBoxLayout *row) {
    card.frame = new QFrame();
    card.frame->setObjectName("cardFrame");
    card.frame->setFrameShape(QFrame::NoFrame);
    auto *lay = new QVBoxLayout(card.frame);
    // V4.2 #2 — vertical padding bumped so descenders ("g" in "Daily avg")
    // don't get clipped against the bottom border.
    lay->setContentsMargins(22, 18, 22, 20);
    lay->setSpacing(6);

    card.caption = new QLabel(caption);
    card.caption->setProperty("class", "caption");
    // V4.2 #2 — caption font set explicitly so it picks up the V4.2 size of
    // 14px (otherwise the QSS [class="caption"] selector kicks in but the
    // widget's preferred size is computed off the inherited 15px base font).
    {
        QFont cf;
        cf.setPointSize(11);  // ~14.7px @ 96 dpi
        cf.setWeight(QFont::Medium);
        card.caption->setFont(cf);
    }
    // Give the label a tiny bit of bottom padding so the descender ("y" in
    // "Daily" and "g" in "avg") has breathing room.
    card.caption->setMinimumHeight(20);
    lay->addWidget(card.caption);

    card.value = new QLabel("—");
    // V4.2 §3: KPI numbers ~40px / 600 with IBM Plex Serif + tabular figures
    QFont nf;
    QString numFam = FontLoader::numericFamily();
    if (!numFam.isEmpty()) nf.setFamily(numFam);
    nf.setPointSize(30);          // ~40px @ 96dpi (up from 24 ≈ 32px)
    nf.setWeight(QFont::DemiBold);
    nf.setStyleStrategy(QFont::PreferAntialias);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    nf.setFeature("tnum", 1);
#endif
    card.value->setFont(nf);
    // V4.2 #2 — ensure descenders aren't clipped (line-height 1.25 of 40px)
    card.value->setMinimumHeight(50);
    lay->addWidget(card.value);

    card.sub = new QLabel();
    card.sub->setProperty("class", "caption");
    {
        QFont sf;
        sf.setPointSize(10);  // ~13px
        card.sub->setFont(sf);
    }
    card.sub->setMinimumHeight(18);
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
    // V4.2 #12 — dark mode KPI numbers explicitly use textPrimary (#F7F7F5),
    // not the muted secondary color that was bleeding through before.
    card.value->setStyleSheet(QString("color:%1;background:transparent;letter-spacing:-0.5px;font-size:40px;font-weight:600;")
                                  .arg(t.textPrimary().name()));
    card.caption->setStyleSheet(QString("color:%1;background:transparent;font-size:14px;font-weight:500;")
                                    .arg(t.textSecondary().name()));
    card.frame->setMinimumWidth(150);
    ShadowEffect::apply(card.frame, ShadowEffect::Card, t.mode() == Theme::Dark);
}

void StatsCardsWidget::applyTheme() {
    applyCardStyle(m_totalCard);
    applyCardStyle(m_countCard);
    applyCardStyle(m_avgCard);
    applyCardStyle(m_peakCard);
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
    m_totalCard.value->setStyleSheet(QString("color:%1;background:transparent;letter-spacing:-0.5px;font-size:40px;font-weight:600;")
                                          .arg(Theme::instance().textPrimary().name()));
}

void StatsCardsWidget::setCount(int count) {
    m_lastCount = count;
    m_countCard.value->setText(QString::number(count));
    m_countCard.value->setStyleSheet(QString("color:%1;background:transparent;letter-spacing:-0.5px;font-size:40px;font-weight:600;")
                                          .arg(Theme::instance().textPrimary().name()));
}

void StatsCardsWidget::setDailyAvg(qint64 minutes) {
    m_lastAvg = minutes;
    m_avgCard.value->setText(formatMinutes(minutes));
    m_avgCard.value->setStyleSheet(QString("color:%1;background:transparent;letter-spacing:-0.5px;font-size:40px;font-weight:600;")
                                        .arg(Theme::instance().textPrimary().name()));
    auto &t = Theme::instance();
    if (minutes < 60) {
        m_avgCard.sub->setText(I18n::t("kpi.avg.low"));
        m_avgCard.sub->setStyleSheet(QString("color:%1;font-size:14px;").arg(t.textPlaceholder().name()));
    } else if (minutes > 480) {
        m_avgCard.sub->setText(I18n::t("kpi.avg.high"));
        m_avgCard.sub->setStyleSheet(QString("color:%1;font-size:14px;").arg(t.danger().name()));
    } else {
        m_avgCard.sub->setText(I18n::t("kpi.avg.normal"));
        m_avgCard.sub->setStyleSheet(QString("color:%1;font-size:14px;").arg(t.success().name()));
    }
}

void StatsCardsWidget::setPeakDay(const QDate &date) {
    m_lastPeak = date;
    if (!date.isValid()) {
        m_peakCard.value->setText("—");
        m_peakCard.value->setStyleSheet(QString("color:%1;background:transparent;letter-spacing:-0.5px;font-size:40px;font-weight:600;")
                                              .arg(Theme::instance().textPrimary().name()));
        m_peakCard.sub->setText("");
        return;
    }
    auto &t = Theme::instance();
    m_peakCard.value->setText(QString("%1/%2").arg(date.month()).arg(date.day()));
    m_peakCard.value->setStyleSheet(QString("color:%1;background:transparent;font-weight:600;letter-spacing:-0.5px;font-size:40px;")
                                        .arg(t.brand().name()));

    QString sub;
    if (I18n::instance().isEnglish()) {
        // English short weekday: Mon..Sun
        static const QStringList en = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
        int dow = date.dayOfWeek();   // 1..7
        sub = en.value((dow - 1 + 7) % 7);
    } else {
        // 中文 周一..周日
        static const QStringList zh = {"一","二","三","四","五","六","日"};
        int dow = date.dayOfWeek();
        sub = QStringLiteral("周") + zh.value((dow - 1 + 7) % 7);
    }
    m_peakCard.sub->setText(sub);
    m_peakCard.sub->setStyleSheet(QString("color:%1;font-size:14px;").arg(t.textSecondary().name()));
}

} // namespace timemaster
