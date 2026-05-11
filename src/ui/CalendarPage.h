#pragma once

#include "../core/Types.h"
#include <QWidget>

class QStackedWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QDialog;

namespace timemaster {

class Database;
class DeepSeekClient;
class MonthView;
class TimeGridView;
class EmptyState;

/**
 * V4 calendar page.
 *  · Header (single row): [<] [>] Title [Today]    [Day|Week|Month]  [+ New Event]
 *  · AI parse input moved to a Ctrl+K Spotlight-style QDialog (no longer always-on bar)
 *  · Empty state overlay (EmptyState) when no events exist; the grid is dimmed underneath
 *  · No "← start from here" arrow hint widget (deleted per V4 § 4.1)
 */
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
    void refreshEmptyState();

    void applyTheme();
    void applyLanguage();

    void openCmdKPalette();

private:
    void buildUi();
    void buildCmdKDialog();
    void updateHeader();
    void switchViewWidget();
    void insertTemplate(int which);
    QString monthName(int month) const;

    Database *m_db;
    DeepSeekClient *m_ai;

    CalendarView m_view = CalendarView::Month;
    QDate m_currentDate;
    QList<CalendarEvent> m_events;
    QString m_pendingParseText;

    // header
    QLabel      *m_titleLabel  = nullptr;
    QPushButton *m_btnDay      = nullptr;
    QPushButton *m_btnWeek     = nullptr;
    QPushButton *m_btnMonth    = nullptr;
    QPushButton *m_btnPrev     = nullptr;
    QPushButton *m_btnNext     = nullptr;
    QPushButton *m_btnToday    = nullptr;
    QPushButton *m_btnAdd      = nullptr;

    // Ctrl+K palette
    QDialog     *m_cmdKDialog   = nullptr;
    QLineEdit   *m_parseInput   = nullptr;
    QPushButton *m_parseButton  = nullptr;
    QPushButton *m_historyButton= nullptr;
    QLabel      *m_parseStatus  = nullptr;
    QLabel      *m_cmdKTitle    = nullptr;
    QLabel      *m_cmdKHint     = nullptr;

    QStackedWidget *m_stack     = nullptr;
    MonthView      *m_monthView = nullptr;
    TimeGridView   *m_weekView  = nullptr;
    TimeGridView   *m_dayView   = nullptr;

    EmptyState     *m_emptyState = nullptr;
    QWidget        *m_viewCard   = nullptr;
};

} // namespace timemaster
