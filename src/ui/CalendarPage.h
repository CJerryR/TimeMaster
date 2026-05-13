//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

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
class EmptyState;

/**
 * V4.1 calendar page.
 *  · Row 1 header: [<] [>] Title [Today]    [Day|Week|Month]  [+ New Event]
 *  · Row 2 (always visible): compact AI parse input + [History] button
 *  · Row 3 (expandable, hidden by default): full input + [Parse] + [X] +
 *    status/hint. Triggered by clicking the compact bar OR pressing Ctrl+K.
 *    Auto-collapses on focus loss or X click.
 *  · Empty state overlay (EmptyState) when no events exist.
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

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

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

    void expandAiPanel();
    void collapseAiPanel();

private:
    void buildUi();
    void updateHeader();
    void switchViewWidget();
    void insertTemplate(int which);
    QString monthName(int month) const;
    // V4.2 #5: position the floating Spotlight-style popup below the compact pill.
    void positionAiPopup();

    Database *m_db;
    DeepSeekClient *m_ai;

    CalendarView m_view = CalendarView::Month;
    QDate m_currentDate;
    QList<CalendarEvent> m_events;
    QString m_pendingParseText;

    // header row 1
    QLabel      *m_titleLabel  = nullptr;
    QPushButton *m_btnDay      = nullptr;
    QPushButton *m_btnWeek     = nullptr;
    QPushButton *m_btnMonth    = nullptr;
    QPushButton *m_btnPrev     = nullptr;
    QPushButton *m_btnNext     = nullptr;
    QPushButton *m_btnToday    = nullptr;
    QPushButton *m_btnAdd      = nullptr;

    // row 2: always-visible compact AI bar
    QWidget     *m_aiBar          = nullptr;
    QLineEdit   *m_aiCompactInput = nullptr;
    QPushButton *m_historyButton  = nullptr;

    // row 3: expandable AI panel
    QWidget     *m_aiExpanded   = nullptr;
    QLabel      *m_aiPanelTitle = nullptr;
    QLineEdit   *m_parseInput   = nullptr;
    QPushButton *m_parseButton  = nullptr;
    QPushButton *m_aiCloseBtn   = nullptr;
    QLabel      *m_aiHintLabel  = nullptr;
    QLabel      *m_parseStatus  = nullptr;

    QStackedWidget *m_stack     = nullptr;
    MonthView      *m_monthView = nullptr;
    TimeGridView   *m_weekView  = nullptr;
    TimeGridView   *m_dayView   = nullptr;

    EmptyState     *m_emptyState = nullptr;
    QWidget        *m_viewCard   = nullptr;
};

} // namespace timemaster
