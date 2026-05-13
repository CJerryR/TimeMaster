//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QBoxLayout>
#include <QVBoxLayout>
#include "../../core/Types.h"

namespace timemaster {

class StatsCardsWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatsCardsWidget(QWidget *parent = nullptr);

    void setTotal(qint64 minutes);
    void setCount(int count);
    void setDailyAvg(qint64 minutes);
    void setPeakDay(const QDate &date);

private slots:
    void applyTheme();

private:
    struct Card {
        QFrame *frame = nullptr;
        QLabel *caption = nullptr;
        QLabel *value = nullptr;
        QLabel *sub = nullptr;
    };
    Card m_totalCard;
    Card m_countCard;
    Card m_avgCard;
    Card m_peakCard;

    // 记下最近一次设置的内容，主题切换后重新应用样式
    qint64 m_lastTotal = 0;
    int    m_lastCount = 0;
    qint64 m_lastAvg = 0;
    QDate  m_lastPeak;

    void setupCard(Card &card, const QString &caption, QBoxLayout *row);
    void applyCardStyle(Card &card);
    QString formatMinutes(qint64 min) const;
};

} // namespace timemaster
