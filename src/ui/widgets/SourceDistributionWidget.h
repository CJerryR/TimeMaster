//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>

class QLabel;

namespace timemaster {

// 来源分布条形绘制器
class SourceBar : public QWidget {
    Q_OBJECT
public:
    // 构造函数：设置最小高度
    explicit SourceBar(QWidget *parent = nullptr);
    // 设置手动/AI/导入三类事件数
    void setSources(int manual, int ai, int imported);

protected:
    // 自绘事件：绘制三段彩色条 + 百分比图例
    void paintEvent(QPaintEvent *) override;

private:
    int m_manual = 0, m_ai = 0, m_import = 0;
    // 绘制单段条形（左端圆角 = 全圆，后续段仅右侧圆角）
    void drawSegment(QPainter &p, int x, int y, int w, int h, QColor c, bool left);
    // 绘制图例：色块 + 文字说明
    void drawLegend(QPainter &p, int x, int y, QColor c, const QString &txt);
};

// 事件来源分布：手动/AI解析/聊天 三段的横向进度条+图例
class SourceDistributionWidget : public QWidget {
    Q_OBJECT
public:
    // 构造函数：创建标题和来源条形组件
    explicit SourceDistributionWidget(QWidget *parent = nullptr);
    // 设置三类来源数据并转发给条形组件
    void setSources(int manual, int ai, int imported);

private:
    QLabel *m_title;
    SourceBar *m_bar;
};

} // namespace timemaster
