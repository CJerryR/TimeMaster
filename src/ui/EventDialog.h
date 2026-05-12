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
    void applyTheme();

private:
    void buildUi();
    void refreshColorButtons();
    void refreshCategoryButtons();
    void refreshPriorityButtons();
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
