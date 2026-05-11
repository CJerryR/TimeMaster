#pragma once

#include "../core/Types.h"
#include <QWidget>

class QStackedWidget;
class QLabel;
class QLineEdit;
class QPushButton;

namespace timemaster {

class Database;
class DeepSeekClient;
class MonthView;
class TimeGridView;

class CalendarPage : public QWidget {
    Q_OBJECT
public:
    explicit CalendarPage(Database *db, DeepSeekClient *ai, QWidget *parent = nullptr);

    void setView(CalendarView v);
    CalendarView currentView() const { return m_view; }

    void goToday();
    void goNext();
    void goPrev();
    void openCreateDialog(const QDateTime &dt = QDateTime());

public slots:
    void refresh();

private slots:
    void onParseClicked();
    void onParseFinished(const QList<ScheduleSuggestion> &items);
    void onParseError(const QString &msg);
    void onEventClicked(const CalendarEvent &event);
    void onTimeSlotClicked(const QDateTime &dt);
    void onMonthDateClicked(const QDate &d);
    void onMonthOverflowClicked(const QDate &d, const QList<CalendarEvent> &events);
    void onHistoryClicked();
    void refreshEmptyHint();

    void applyTheme();

private:
    void buildUi();
    void updateHeader();
    void switchViewWidget();

    Database *m_db;
    DeepSeekClient *m_ai;

    CalendarView m_view = CalendarView::Month;
    QDate m_currentDate;
    QList<CalendarEvent> m_events;
    QString m_pendingParseText;

    // header
    QLabel *m_titleLabel;
    QPushButton *m_btnDay;
    QPushButton *m_btnWeek;
    QPushButton *m_btnMonth;
    QPushButton *m_btnPrev = nullptr;
    QPushButton *m_btnNext = nullptr;

    // ai input
    QLabel *m_sparkleIcon = nullptr;
    QLineEdit *m_parseInput;
    QPushButton *m_parseButton;
    QPushButton *m_historyButton;
    QLabel *m_parseStatus;
    QLabel *m_emptyHint = nullptr;

    QStackedWidget *m_stack;
    MonthView *m_monthView;
    TimeGridView *m_weekView;
    TimeGridView *m_dayView;
};

} // namespace timemaster
