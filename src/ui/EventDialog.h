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

namespace timemaster {

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
    QString m_aiBatchId;
    EventSource m_source = EventSource::Manual;
    QDateTime m_createdAt;

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

} // namespace timemaster
