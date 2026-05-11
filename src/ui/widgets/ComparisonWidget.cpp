#include "ComparisonWidget.h"
#include "../Theme.h"
#include "../../core/Database.h"
#include "../../core/I18n.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QFont>
#include <QDateTime>

namespace timemaster {

static QFrame *makeMiniCard(QLabel **valueOut, QLabel **captionOut) {
    auto *f = new QFrame;
    f->setObjectName("CompCard");
    auto *lay = new QVBoxLayout(f);
    lay->setContentsMargins(18, 14, 18, 14);
    lay->setSpacing(2);

    auto *cap = new QLabel();
    cap->setObjectName("CompCaption");
    lay->addWidget(cap);

    auto *val = new QLabel("—");
    val->setObjectName("CompValue");
    QFont vf; vf.setPointSize(18); vf.setWeight(QFont::DemiBold);
    val->setFont(vf);
    lay->addWidget(val);

    if (valueOut) *valueOut = val;
    if (captionOut) *captionOut = cap;
    return f;
}

ComparisonWidget::ComparisonWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(10);

    m_title = new QLabel();
    m_title->setProperty("class", "subtitle");
    root->addWidget(m_title);

    auto *row = new QHBoxLayout;
    row->setSpacing(10);

    m_pastCard = new QFrame;
    m_pastCard->setObjectName("CompPanePast");
    auto *pastLay = new QVBoxLayout(m_pastCard);
    pastLay->setContentsMargins(16, 14, 16, 14);
    pastLay->setSpacing(8);
    m_pastHeader = new QLabel();
    m_pastHeader->setObjectName("CompHeader");
    pastLay->addWidget(m_pastHeader);
    auto *pastRow = new QHBoxLayout;
    pastRow->setSpacing(8);
    pastRow->addWidget(makeMiniCard(&m_pastCountVal, &m_pastCap1));
    pastRow->addWidget(makeMiniCard(&m_pastHoursVal, &m_pastCap2));
    pastLay->addLayout(pastRow);
    row->addWidget(m_pastCard, 1);

    m_futCard = new QFrame;
    m_futCard->setObjectName("CompPaneFuture");
    auto *futLay = new QVBoxLayout(m_futCard);
    futLay->setContentsMargins(16, 14, 16, 14);
    futLay->setSpacing(8);
    m_futHeader = new QLabel();
    m_futHeader->setObjectName("CompHeader");
    futLay->addWidget(m_futHeader);
    auto *futRow = new QHBoxLayout;
    futRow->setSpacing(8);
    futRow->addWidget(makeMiniCard(&m_futCountVal, &m_futCap1));
    futRow->addWidget(makeMiniCard(&m_futHoursVal, &m_futCap2));
    futLay->addLayout(futRow);
    row->addWidget(m_futCard, 1);

    root->addLayout(row);

    m_deltaLabel = new QLabel;
    m_deltaLabel->setObjectName("CompDelta");
    m_deltaLabel->setAlignment(Qt::AlignCenter);
    m_deltaLabel->setMinimumHeight(28);
    root->addWidget(m_deltaLabel);

    connect(&Theme::instance(), &Theme::changed,        this, &ComparisonWidget::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, &ComparisonWidget::applyLanguage);

    applyLanguage();
    applyTheme();
}

void ComparisonWidget::applyLanguage() {
    if (m_title)      m_title->setText(I18n::t("widget.comparison"));
    if (m_pastHeader) m_pastHeader->setText(I18n::t("widget.past_header"));
    if (m_futHeader)  m_futHeader->setText(I18n::t("widget.future_header"));
    if (m_pastCap1)   m_pastCap1->setText(I18n::t("widget.events"));
    if (m_pastCap2)   m_pastCap2->setText(I18n::t("widget.duration"));
    if (m_futCap1)    m_futCap1->setText(I18n::t("widget.events"));
    if (m_futCap2)    m_futCap2->setText(I18n::t("widget.duration"));
    refresh();
}

void ComparisonWidget::refresh() {
    if (!m_db) return;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime past7Start(QDate::currentDate().addDays(-7), QTime(0, 0));
    QDateTime past7End = now;
    QDateTime fut7Start = now;
    QDateTime fut7End(QDate::currentDate().addDays(7), QTime(23, 59, 59));

    auto past = m_db->getEventsByRange(past7Start, past7End);
    auto fut = m_db->getEventsByRange(fut7Start, fut7End);

    qint64 pastMin = 0, futMin = 0;
    int pastCnt = 0, futCnt = 0;
    for (const auto &e : past) {
        if (e.allDay) { pastCnt++; continue; }
        qint64 lo = qMax(e.startDate.toMSecsSinceEpoch(), past7Start.toMSecsSinceEpoch());
        qint64 hi = qMin(e.endDate.toMSecsSinceEpoch(), past7End.toMSecsSinceEpoch());
        if (hi > lo) { pastMin += (hi - lo) / 60000; pastCnt++; }
    }
    for (const auto &e : fut) {
        if (e.allDay) { futCnt++; continue; }
        qint64 lo = qMax(e.startDate.toMSecsSinceEpoch(), fut7Start.toMSecsSinceEpoch());
        qint64 hi = qMin(e.endDate.toMSecsSinceEpoch(), fut7End.toMSecsSinceEpoch());
        if (hi > lo) { futMin += (hi - lo) / 60000; futCnt++; }
    }

    auto fmt = [](qint64 m) -> QString {
        if (m <= 0) return "0";
        qint64 h = m / 60, mm = m % 60;
        if (h == 0) return QString("%1m").arg(mm);
        if (mm == 0) return QString("%1h").arg(h);
        return QString("%1h %2m").arg(h).arg(mm);
    };

    m_pastCountVal->setText(QString::number(pastCnt));
    m_pastHoursVal->setText(fmt(pastMin));
    m_futCountVal->setText(QString::number(futCnt));
    m_futHoursVal->setText(fmt(futMin));

    auto &t = Theme::instance();
    QString deltaText;
    QString color;
    if (pastMin == 0 && futMin == 0) {
        deltaText = I18n::t("widget.delta.easy");
        color = t.textSecondary().name();
    } else if (futMin > pastMin * 1.2) {
        qint64 diff = futMin - pastMin;
        deltaText = I18n::t("widget.delta.more_fmt").arg(fmt(diff));
        color = t.brand().name();
    } else if (futMin < pastMin * 0.8) {
        qint64 diff = pastMin - futMin;
        deltaText = I18n::t("widget.delta.less_fmt").arg(fmt(diff));
        color = t.success().name();
    } else {
        deltaText = I18n::t("widget.delta.stable");
        color = t.textSecondary().name();
    }
    m_deltaLabel->setText(deltaText);
    m_deltaLabel->setStyleSheet(QString("color:%1;font-size:13px;font-weight:500;").arg(color));
}

void ComparisonWidget::applyTheme() {
    auto &t = Theme::instance();
    QString cardBg = t.bgContainer().name();
    QString strokeR = t.strokeRgba();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();

    setStyleSheet(QString(R"(
        QFrame#CompPanePast, QFrame#CompPaneFuture {
            background: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }
        QFrame#CompPaneFuture {
            background: %5;
        }
        QLabel#CompHeader {
            color: %3;
            font-size: 13px;
            font-weight: 600;
        }
        QFrame#CompCard {
            background: %1;
            border: 1px solid %2;
            border-radius: 8px;
        }
        QFrame#CompPaneFuture QFrame#CompCard {
            background: %1;
        }
        QLabel#CompValue {
            color: %3;
            background: transparent;
        }
        QLabel#CompCaption {
            color: %4;
            font-size: 12px;
        }
    )")
    .arg(cardBg)
    .arg(strokeR)
    .arg(textPrim)
    .arg(textSec)
    .arg(t.brandLight().name()));
}

} // namespace timemaster
