// ============================================================
// InsightsWidget.h
// 智能洞察文字列表（不依赖 AI，本地基于 stats 推导）
// 每条前面带一个品牌色小圆点
// ============================================================
#pragma once
#include <QWidget>
#include <QStringList>

namespace cadence {

class InsightsWidget : public QWidget {
    Q_OBJECT
public:
    explicit InsightsWidget(QWidget* parent = nullptr);
    void setInsights(const QStringList& items);
    bool isEmpty() const { return m_items.isEmpty(); }

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QStringList m_items;
    int computeHeight() const;
};

} // namespace cadence
