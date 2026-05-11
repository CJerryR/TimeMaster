#pragma once

#include <QWidget>
#include <QLabel>

namespace timemaster {

/**
 * 数据来源分布：手动 / AI 解析 / AI 对话。
 * 顶部标题 + 一根堆叠条 + 三条 legend。
 */
class SourceDistributionWidget : public QWidget {
    Q_OBJECT
public:
    explicit SourceDistributionWidget(QWidget *parent = nullptr);
    void setSources(int manualCount, int aiParseCount, int chatCount);

    QSize sizeHint() const override { return QSize(360, 220); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void applyTheme();

    QLabel *m_title = nullptr;
    QLabel *m_subtitle = nullptr;

    int m_manual = 0;
    int m_aiParse = 0;
    int m_chat = 0;
};

} // namespace timemaster
