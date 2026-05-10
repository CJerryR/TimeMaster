// ============================================================
// StatsCardsWidget.h
// 4 张概览卡片：总日程数 / 总时长 / 日均日程 / 紧急日程
// 每张支持环比显示（↑ 12% 红色 / ↓ 8% 绿色 / 持平 灰色 / 新增 绿色）
// ============================================================
#pragma once
#include <QWidget>
#include <QString>
#include <QVector>

class QLabel;

namespace cadence {

struct StatCard {
    QString icon;     // emoji 或文字
    QString value;    // "42" / "3h 20m"
    QString label;    // "总日程数"
    QString delta;    // "↑ 12%" 或空字符串表示无环比
    QString deltaColor; // "#dc2626" 红 / "#16a34a" 绿 / 空
};

class StatsCardsWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatsCardsWidget(QWidget* parent = nullptr);
    void setCards(const QVector<StatCard>& cards);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QVector<StatCard> m_cards;
};

} // namespace cadence
