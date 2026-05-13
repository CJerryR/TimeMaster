//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QDialog>
#include <QList>

class QVBoxLayout;
class QScrollArea;
class QLabel;
class QPushButton;
class QFrame;

namespace timemaster {

class AiSuggestionRow;

/**
 * AI 识别结果对话框
 *  - 每条解析结果一行：可勾选导入、可编辑、可删除
 *  - 编辑后，列表中的预览随之刷新
 *  - 点击「导入」时，只导入勾选中的条目
 */
// AI 解析结果弹窗：逐条展示日程建议，支持勾选/修改/删除/全选后批量导入
class AiResultsDialog : public QDialog {
    Q_OBJECT
public:
    // 构造函数
    explicit AiResultsDialog(QList<ScheduleSuggestion> items, QWidget *parent = nullptr);

    // 获取用户勾选的所有日程建议
    QList<ScheduleSuggestion> selectedSuggestions() const;

private slots:
    // 应用主题样式
    void applyTheme();
    // 编辑某行建议：弹出 EventDialog 修改
    void onEditRow(int index);
    // 删除某行建议
    void onDeleteRow(int index);
    // 全选/全不选切换
    void onToggleAll();

private:
    // 构建界面：标题 + 统计 + 全选按钮 + 滚动列表 + 确认按钮
    void buildUi();
    // 重建所有行：根据当前 items 重新生成行控件
    void rebuildRows();
    // 创建单行控件
    QFrame *makeRow(int index);

    QList<ScheduleSuggestion> m_items;
    QList<bool>               m_checked;     // 与 m_items 同步
    QList<AiSuggestionRow*>   m_rowWidgets;  // 当前显示的行

    QVBoxLayout *m_rowsLayout = nullptr;
    QScrollArea *m_scroll = nullptr;
    QLabel      *m_summaryLabel = nullptr;
    QPushButton *m_toggleAllBtn = nullptr;
};

} // namespace timemaster
