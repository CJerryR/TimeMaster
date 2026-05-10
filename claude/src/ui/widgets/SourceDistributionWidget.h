// ============================================================
// SourceDistributionWidget.h
// 录入方式：手动数值 + AI 数值 + 一条 AI 占比进度条
// ============================================================
#pragma once
#include <QWidget>

namespace cadence {

class SourceDistributionWidget : public QWidget {
    Q_OBJECT
public:
    explicit SourceDistributionWidget(QWidget* parent = nullptr);
    void setData(int manualCount, int aiCount);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    int m_manual{0};
    int m_ai{0};
};

} // namespace cadence
