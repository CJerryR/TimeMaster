// ============================================================
// RhythmCardWidget.h
// 简单的小标签 + 大数值卡片
// 用作"最忙的一天" / "高峰时段"两张并排卡
// ============================================================
#pragma once
#include <QWidget>
#include <QString>

class QLabel;

namespace cadence {

class RhythmCardWidget : public QWidget {
    Q_OBJECT
public:
    explicit RhythmCardWidget(const QString& title, QWidget* parent = nullptr);
    void setValue(const QString& v);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QString m_title;
    QString m_value{"—"};
};

} // namespace cadence
