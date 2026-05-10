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

namespace timeplan {

/**
 * 新建/编辑事件
 * - 创建模式：传入选中日期
 * - 编辑模式：传入已有事件
 * 优化点：
 *   - 颜色选择改为 12 个圆形色块，点击切换，比原版下拉选择器直观
 *   - 类别 + 优先级用胶囊式 toggle button
 *   - 全天勾选时自动隐藏时间字段
 */
class EventDialog : public QDialog {
    Q_OBJECT
public:
    explicit EventDialog(QWidget *parent = nullptr);

    void setupForCreate(const QDateTime &defaultStart);
    void setupForEdit(const CalendarEvent &event);

    CalendarEvent result() const;

signals:
    void requestDelete(const QString &id);

private slots:
    void onAllDayToggled(bool checked);
    void onCategoryClicked(int idx);
    void onPriorityClicked(int idx);
    void onColorClicked(int idx);
    void onSubmit();
    void onDeleteClicked();

private:
    void buildUi();
    void applyTheme();
    void refreshColorButtons();
    void refreshCategoryButtons();
    void refreshPriorityButtons();
    void syncEndDateMin();

    bool m_isEditing = false;
    QString m_eventId;
    QDateTime m_createdAt;

    // UI
    QLineEdit *m_titleEdit;
    QLineEdit *m_locationEdit;
    QPlainTextEdit *m_descriptionEdit;
    QDateTimeEdit *m_startEdit;
    QDateTimeEdit *m_endEdit;
    QCheckBox *m_allDayCheck;
    QSpinBox *m_reminderSpin;

    QList<QPushButton *> m_categoryButtons;
    QList<QPushButton *> m_priorityButtons;
    QList<QPushButton *> m_colorButtons;

    QPushButton *m_submitButton;
    QPushButton *m_cancelButton;
    QPushButton *m_deleteButton;

    EventCategory m_selectedCategory = EventCategory::Work;
    EventPriority m_selectedPriority = EventPriority::Normal;
    EventColor m_selectedColor = EventColor::Blue;
};

} // namespace timeplan
