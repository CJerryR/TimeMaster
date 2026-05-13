//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

// V4.3 #4 — Qt 经典 FlowLayout（基于 Qt 官方示例的实现简化版）：
// 类别按钮当数量超过一行宽度时自动换行。EventDialog 的 category row 用它替换
// 原来的 QHBoxLayout。

#include <QLayout>
#include <QRect>
#include <QStyle>

namespace timemaster {

// 流式布局：自动换行的水平布局，类似 CSS flex-wrap，用于类别/颜色按钮等可变数量的标签组
class FlowLayout : public QLayout {
    Q_OBJECT
public:
    // 构造函数：指定父窗口和间距参数
    explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    // 构造函数：纯间距参数
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    // 析构函数：删除所有布局项
    ~FlowLayout() override;
    // 添加布局项
    void addItem(QLayoutItem *item) override;
    // 获取水平间距
    int horizontalSpacing() const;
    // 获取垂直间距
    int verticalSpacing() const;
    // 返回扩展方向（从不扩展）
    Qt::Orientations expandingDirections() const override;
    // 是否根据宽度计算高度
    bool hasHeightForWidth() const override { return true; }
    // 根据宽度计算所需高度
    int heightForWidth(int) const override;
    // 返回布局项数量
    int count() const override;
    // 返回指定索引的布局项
    QLayoutItem *itemAt(int index) const override;
    // 返回最小尺寸
    QSize minimumSize() const override;
    // 设置布局几何并重新排列子项
    void setGeometry(const QRect &rect) override;
    // 返回建议尺寸
    QSize sizeHint() const override;
    // 移除并返回指定索引的布局项
    QLayoutItem *takeAt(int index) override;

private:
    // 核心布局算法：逐行排列子项，超宽自动换行
    int doLayout(const QRect &rect, bool testOnly) const;
    // 获取智能间距（继承父控件样式默认值）
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> m_items;
    int m_hSpace;
    int m_vSpace;
};

} // namespace timemaster
