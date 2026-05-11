#include "ComparisonWidget.h"
#include "../Theme.h"
#include "../../core/Database.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QFont>
#include <QDateTime>

namespace timemaster {

static QFrame *makeMiniCard(QLabel **valueOut, QLabel **captionOut,
                            const QString &caption) {
    auto *f = new QFrame;
    f->setObjectName("CompCard");
    auto *lay = new QVBoxLayout(f);
    lay->setContentsMargins(18, 14, 18, 14);
    lay->setSpacing(2);

    auto *cap = new QLabel(caption);
    cap->setObjectName("CompCaption");
    lay->addWidget(cap);

    auto *val = new QLabel("—");
    val->setObjectName("CompValue");
    QFont vf; vf.setPointSize(20); vf.setWeight(QFont::Bold);
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

    m_title = new QLabel("过去 / 未来 一周对比");
    m_title->setProperty("class", "subtitle");
    root->addWidget(m_title);

    auto *row = new QHBoxLayout;
    row->setSpacing(10);

    // 过去
    m_pastCard = new QFrame;
    m_pastCard->setObjectName("CompPanePast");
    auto *pastLay = new QVBoxLayout(m_pastCard);
    pastLay->setContentsMargins(16, 14, 16, 14);
    pastLay->setSpacing(8);
    auto *pastHeader = new QLabel("过去 7 天 · 已度过");
    pastHeader->setObjectName("CompHeader");
    pastLay->addWidget(pastHeader);
    auto *pastRow = new QHBoxLayout;
    pastRow->setSpacing(8);
    QLabel *pastCap1, *pastCap2;
    pastRow->addWidget(makeMiniCard(&m_pastCountVal, &pastCap1, "事件"));
    pastRow->addWidget(makeMiniCard(&m_pastHoursVal, &pastCap2, "时长"));
    pastLay->addLayout(pastRow);
    row->addWidget(m_pastCard, 1);

    // 未来
    m_futCard = new QFrame;
    m_futCard->setObjectName("CompPaneFuture");
    auto *futLay = new QVBoxLayout(m_futCard);
    futLay->setContentsMargins(16, 14, 16, 14);
    futLay->setSpacing(8);
    auto *futHeader = new QLabel("未来 7 天 · 待迎接");
    futHeader->setObjectName("CompHeader");
    futLay->addWidget(futHeader);
    auto *futRow = new QHBoxLayout;
    futRow->setSpacing(8);
    QLabel *futCap1, *futCap2;
    futRow->addWidget(makeMiniCard(&m_futCountVal, &futCap1, "事件"));
    futRow->addWidget(makeMiniCard(&m_futHoursVal, &futCap2, "时长"));
    futLay->addLayout(futRow);
    row->addWidget(m_futCard, 1);

    root->addLayout(row);

    m_deltaLabel = new QLabel;
    m_deltaLabel->setObjectName("CompDelta");
    m_deltaLabel->setAlignment(Qt::AlignCenter);
    m_deltaLabel->setMinimumHeight(28);
    root->addWidget(m_deltaLabel);

    connect(&Theme::instance(), &Theme::changed, this, &ComparisonWidget::applyTheme);
    applyTheme();
}

void ComparisonWidget::refresh() {
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

    // 节奏对比
    auto &t = Theme::instance();
    QString deltaText;
    QString color;
    if (pastMin == 0 && futMin == 0) {
        deltaText = "近期都很轻松，做点自己喜欢的事吧";
        color = t.textSecondary().name();
    } else if (futMin > pastMin * 1.2) {
        qint64 diff = futMin - pastMin;
        deltaText = QString("下一周比过去多 %1，准备好节奏切换").arg(fmt(diff));
        color = t.brand().name();
    } else if (futMin < pastMin * 0.8) {
        qint64 diff = pastMin - futMin;
        deltaText = QString("下一周比过去少 %1，余出来的时间可以做点什么？").arg(fmt(diff));
        color = t.success().name();
    } else {
        deltaText = "节奏稳定，保持。";
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
            font-size: 11px;
        }
    )")
    .arg(cardBg)
    .arg(strokeR)
    .arg(textPrim)
    .arg(textSec)
    .arg(t.brandLight().name()));
}

} // namespace timemaster
