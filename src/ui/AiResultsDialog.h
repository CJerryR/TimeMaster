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
class AiResultsDialog : public QDialog {
    Q_OBJECT
public:
    explicit AiResultsDialog(QList<ScheduleSuggestion> items, QWidget *parent = nullptr);

    // 用户确认导入的、勾选项 + 已编辑后的列表
    QList<ScheduleSuggestion> selectedSuggestions() const;

private slots:
    void applyTheme();
    void onEditRow(int index);
    void onDeleteRow(int index);
    void onToggleAll();

private:
    void buildUi();
    void rebuildRows();
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
