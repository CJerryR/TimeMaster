//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "FlowLayout.h"

#include <QWidget>
#include <QSize>

namespace timemaster {

// 构造函数：指定父窗口和间距
FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

// 构造函数：仅间距参数，无父窗口
FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

// 析构函数：清理所有布局项
FlowLayout::~FlowLayout() {
    QLayoutItem *item;
    while ((item = takeAt(0))) delete item;
}

// 追加一个布局项到列表末尾
void FlowLayout::addItem(QLayoutItem *item) { m_items.append(item); }

// 返回水平间距（未设定则从父控件样式获取默认值）
int FlowLayout::horizontalSpacing() const {
    if (m_hSpace >= 0) return m_hSpace;
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

// 返回垂直间距（未设定则从父控件样式获取默认值）
int FlowLayout::verticalSpacing() const {
    if (m_vSpace >= 0) return m_vSpace;
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

// 返回当前布局项总数
int FlowLayout::count() const { return m_items.size(); }

// 返回指定索引的布局项
QLayoutItem *FlowLayout::itemAt(int index) const {
    return m_items.value(index);
}

// 移除并返回指定索引的布局项
QLayoutItem *FlowLayout::takeAt(int index) {
    if (index >= 0 && index < m_items.size()) return m_items.takeAt(index);
    return nullptr;
}

// 返回扩展方向（水平垂直均不扩展）
Qt::Orientations FlowLayout::expandingDirections() const {
    return {};
}

// 根据宽度计算所需高度（委托给 doLayout）
int FlowLayout::heightForWidth(int width) const {
    return doLayout(QRect(0, 0, width, 0), true);
}

// 设置布局几何区域并执行实际排列
void FlowLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

// 建议尺寸 = 最小尺寸
QSize FlowLayout::sizeHint() const {
    return minimumSize();
}

// 最小尺寸 = 所有子项最小尺寸的并集 + 边距
QSize FlowLayout::minimumSize() const {
    QSize size;
    for (auto *item : m_items) {
        size = size.expandedTo(item->minimumSize());
    }
    const QMargins m = contentsMargins();
    size += QSize(m.left() + m.right(), m.top() + m.bottom());
    return size;
}

// 核心换行布局：从左到右排列，超宽则换行
int FlowLayout::doLayout(const QRect &rect, bool testOnly) const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (auto *item : m_items) {
        QWidget *w = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1 && w)
            spaceX = w->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1 && w)
            spaceY = w->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }
        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

// 获取父控件的默认间距值
int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const {
    QObject *parent = this->parent();
    if (!parent) return -1;
    if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(parent)->spacing();
}

} // namespace timemaster
