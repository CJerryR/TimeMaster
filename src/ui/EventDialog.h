//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QDialog>

class QLineEdit;
class QPlainTextEdit;
class QDateTimeEdit;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QButtonGroup;
class QPushButton;
class QLabel;

namespace timemaster {

class FlowLayout;

// 日程编辑弹窗：标题/时间/全天/类别/优先级/颜色/地点/提醒/备注，支持创建和编辑两种模式
class EventDialog : public QDialog {
    Q_OBJECT
public:
    // 构造函数
    explicit EventDialog(QWidget *parent = nullptr);

    // 设为创建模式，填入默认起始时间
    void setupForCreate(const QDateTime &defaultStart);
    // 设为编辑模式，用已有事件数据填充各字段
    void setupForEdit(const CalendarEvent &event);

    // 获取弹窗内编辑好的日程对象
    CalendarEvent result() const;

signals:
    // 请求删除事件信号（编辑模式删除按钮触发）
    void requestDelete(const QString &id);

private slots:
    // 全天开关切换：调整时间格式显示
    void onAllDayToggled(bool checked);
    // 类别按钮点击：更新选中类别和默认颜色
    void onCategoryClicked(int idx);
    // 优先级按钮点击：更新选中优先级
    void onPriorityClicked(int idx);
    // 颜色按钮点击：更新选中颜色
    void onColorClicked(int idx);
    // 保存按钮：校验必填项后接受弹窗
    void onSubmit();
    // 删除按钮：确认后发送删除信号
    void onDeleteClicked();
    // 应用主题样式：刷新全局 QSS 和按钮状态
    void applyTheme();

private:
    // 构建 UI 布局：组装所有字段和操作按钮
    void buildUi();
    // 刷新颜色按钮选中态和样式
    void refreshColorButtons();
    // 刷新类别按钮选中态和样式
    void refreshCategoryButtons();
    // 刷新优先级按钮选中态和样式
    void refreshPriorityButtons();
    // 同步结束时间下限：确保结束不早于开始
    void syncEndDateMin();

    bool m_isEditing = false;
    QString m_eventId;
    QString m_aiBatchId;
    EventSource m_source = EventSource::Manual;
    QDateTime m_createdAt;

    QLabel *m_titleLabel = nullptr;          // V4.3 #2 — 显式持有以便主题切换刷新
    QLineEdit *m_titleEdit = nullptr;
    QLineEdit *m_locationEdit = nullptr;
    QPlainTextEdit *m_descriptionEdit = nullptr;
    QDateTimeEdit *m_startEdit = nullptr;
    QDateTimeEdit *m_endEdit = nullptr;
    QCheckBox *m_allDayCheck = nullptr;
    QSpinBox *m_reminderSpin = nullptr;

    QList<QPushButton *> m_categoryButtons;
    QList<QPushButton *> m_priorityButtons;
    QList<QPushButton *> m_colorButtons;

    FlowLayout *m_categoryFlow = nullptr;     // V4.3 #4
    FlowLayout *m_colorFlow    = nullptr;

    QPushButton *m_submitButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QPushButton *m_deleteButton = nullptr;

    EventCategory m_selectedCategory = EventCategory::Work;
    EventPriority m_selectedPriority = EventPriority::Normal;
    EventColor m_selectedColor = EventColor::Blue;
};

} // namespace timemaster
