#pragma once

#include <QWidget>
#include <QLabel>
#include <QDate>
#include <QFrame>

namespace timemaster {

/**
 * 统计页顶部的 4 张概览卡：
 *  - 总时长
 *  - 事件数
 *  - 日均时长
 *  - 高峰日
 *
 * 半透明卡片样式，跟随 Theme 切换。
 */
class StatsCardsWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatsCardsWidget(QWidget *parent = nullptr);

    void setTotal(qint64 minutes);
    void setCount(int n);
    void setDailyAvg(qint64 minutes);
    void setPeakDay(const QDate &d);

private:
    struct Card {
        QFrame *frame = nullptr;
        QLabel *icon = nullptr;
        QLabel *label = nullptr;
        QLabel *value = nullptr;
        QLabel *sub = nullptr;
    };
    Card makeCard(const QString &icon, const QString &label, const QString &accent);
    void applyTheme();
    QString formatMinutes(qint64 m) const;

    Card m_totalCard;
    Card m_countCard;
    Card m_avgCard;
    Card m_peakCard;
};

} // namespace timemaster
