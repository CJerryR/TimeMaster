#include "CalendarPage.h"
#include "Theme.h"
#include "MonthView.h"
#include "TimeGridView.h"
#include "EventDialog.h"
#include "../core/Database.h"
#include "../core/DeepSeekClient.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QShortcut>
#include <QKeySequence>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialog>
#include <QDialogButtonBox>

namespace timeplan {

CalendarPage::CalendarPage(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QWidget(parent), m_db(db), m_ai(ai)
{
    m_currentDate = QDate::currentDate();
    buildUi();
    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, &CalendarPage::applyTheme);

    connect(m_db, &Database::eventsChanged, this, &CalendarPage::refresh);
    connect(m_ai, &DeepSeekClient::parseFinished, this, &CalendarPage::onParseFinished);
    connect(m_ai, &DeepSeekClient::parseError, this, &CalendarPage::onParseError);

    refresh();
}

void CalendarPage::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ============ 顶部 AI 解析输入区 ============
    auto *parseRow = new QWidget(this);
    auto *parseLayout = new QHBoxLayout(parseRow);
    parseLayout->setContentsMargins(20, 16, 20, 8);
    parseLayout->setSpacing(10);

    auto *icon = new QLabel("🤖");
    QFont iconFont; iconFont.setPointSize(16);
    icon->setFont(iconFont);
    parseLayout->addWidget(icon);

    m_parseInput = new QLineEdit();
    m_parseInput->setPlaceholderText("粘贴日程文本，AI 自动识别…  例：明天下午3点项目评审、周三上午健身");
    m_parseInput->setMinimumHeight(38);
    parseLayout->addWidget(m_parseInput, 1);

    m_parseButton = new QPushButton("解析");
    m_parseButton->setProperty("class", "primary");
    m_parseButton->setMinimumSize(90, 38);
    m_parseButton->setCursor(Qt::PointingHandCursor);
    parseLayout->addWidget(m_parseButton);

    m_parseStatus = new QLabel();
    m_parseStatus->setProperty("class", "caption");
    parseLayout->addWidget(m_parseStatus);

    root->addWidget(parseRow);

    connect(m_parseButton, &QPushButton::clicked, this, &CalendarPage::onParseClicked);
    connect(m_parseInput, &QLineEdit::returnPressed, this, &CalendarPage::onParseClicked);

    // ============ 中部头部（标题 + 视图切换） ============
    auto *headerWidget = new QWidget(this);
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 8, 20, 8);
    headerLayout->setSpacing(12);

    auto *btnAdd = new QPushButton("+ 新建日程");
    btnAdd->setProperty("class", "primary");
    btnAdd->setMinimumSize(110, 32);
    btnAdd->setCursor(Qt::PointingHandCursor);
    connect(btnAdd, &QPushButton::clicked, this, [this] { openCreateDialog(); });
    headerLayout->addWidget(btnAdd);

    auto *btnPrev = new QPushButton("‹");
    btnPrev->setProperty("class", "ghost");
    btnPrev->setFixedSize(32, 32);
    btnPrev->setCursor(Qt::PointingHandCursor);
    connect(btnPrev, &QPushButton::clicked, this, &CalendarPage::goPrev);
    headerLayout->addWidget(btnPrev);

    auto *btnNext = new QPushButton("›");
    btnNext->setProperty("class", "ghost");
    btnNext->setFixedSize(32, 32);
    btnNext->setCursor(Qt::PointingHandCursor);
    connect(btnNext, &QPushButton::clicked, this, &CalendarPage::goNext);
    headerLayout->addWidget(btnNext);

    m_titleLabel = new QLabel();
    QFont tf; tf.setPointSize(18); tf.setWeight(QFont::Bold);
    m_titleLabel->setFont(tf);
    m_titleLabel->setMinimumWidth(220);
    headerLayout->addWidget(m_titleLabel);

    auto *btnToday = new QPushButton("今天");
    btnToday->setMinimumSize(64, 30);
    btnToday->setCursor(Qt::PointingHandCursor);
    connect(btnToday, &QPushButton::clicked, this, &CalendarPage::goToday);
    headerLayout->addWidget(btnToday);

    headerLayout->addStretch();

    // 视图切换分段控件
    m_btnDay = new QPushButton("日");
    m_btnWeek = new QPushButton("周");
    m_btnMonth = new QPushButton("月");
    for (auto *b : {m_btnDay, m_btnWeek, m_btnMonth}) {
        b->setCheckable(true);
        b->setMinimumSize(48, 30);
        b->setCursor(Qt::PointingHandCursor);
        headerLayout->addWidget(b);
    }
    connect(m_btnDay,   &QPushButton::clicked, this, [this]{ setView(CalendarView::Day); });
    connect(m_btnWeek,  &QPushButton::clicked, this, [this]{ setView(CalendarView::Week); });
    connect(m_btnMonth, &QPushButton::clicked, this, [this]{ setView(CalendarView::Month); });

    root->addWidget(headerWidget);

    // ============ 视图区域 ============
    m_stack = new QStackedWidget(this);
    m_monthView = new MonthView(this);
    m_weekView = new TimeGridView(TimeGridView::WeekMode, this);
    m_dayView = new TimeGridView(TimeGridView::DayMode, this);
    m_stack->addWidget(m_monthView);
    m_stack->addWidget(m_weekView);
    m_stack->addWidget(m_dayView);
    root->addWidget(m_stack, 1);

    connect(m_monthView, &MonthView::dateClicked,
            this, &CalendarPage::onMonthDateClicked);
    connect(m_monthView, &MonthView::eventClicked,
            this, &CalendarPage::onEventClicked);
    connect(m_monthView, &MonthView::overflowClicked,
            this, &CalendarPage::onMonthOverflowClicked);

    connect(m_weekView, &TimeGridView::timeSlotClicked,
            this, &CalendarPage::onTimeSlotClicked);
    connect(m_weekView, &TimeGridView::eventClicked,
            this, &CalendarPage::onEventClicked);
    connect(m_dayView, &TimeGridView::timeSlotClicked,
            this, &CalendarPage::onTimeSlotClicked);
    connect(m_dayView, &TimeGridView::eventClicked,
            this, &CalendarPage::onEventClicked);

    // ============ 快捷键 ============
    auto bind = [this](const char *key, std::function<void()> fn) {
        auto *sc = new QShortcut(QKeySequence(key), this);
        connect(sc, &QShortcut::activated, this, fn);
    };
    bind("Ctrl+N", [this]{ openCreateDialog(); });
    bind("Meta+N", [this]{ openCreateDialog(); });
    bind("T",      [this]{ goToday(); });
    bind("Left",   [this]{ goPrev(); });
    bind("Right",  [this]{ goNext(); });
    bind("1",      [this]{ setView(CalendarView::Day); });
    bind("2",      [this]{ setView(CalendarView::Week); });
    bind("3",      [this]{ setView(CalendarView::Month); });

    // 默认月视图，并在切换到周/日时滚动到 7 点
    setView(CalendarView::Month);
}

void CalendarPage::applyTheme() {
    auto &t = Theme::instance();
    setStyleSheet(t.globalStylesheet());
    m_titleLabel->setStyleSheet(QString("color:%1;").arg(t.textPrimary().name()));

    auto refreshTab = [this](QPushButton *b, bool active) {
        auto &tt = Theme::instance();
        if (active) {
            b->setStyleSheet(QString(
                "QPushButton{background:%1;color:white;border:none;border-radius:6px;font-weight:600;}"
            ).arg(tt.brand().name()));
        } else {
            b->setStyleSheet(QString(
                "QPushButton{background:transparent;border:none;color:%1;border-radius:6px;}"
                "QPushButton:hover{background:%2;color:%3;}"
            ).arg(tt.textSecondary().name(), tt.bgHover().name(), tt.textPrimary().name()));
        }
    };
    refreshTab(m_btnDay,   m_view == CalendarView::Day);
    refreshTab(m_btnWeek,  m_view == CalendarView::Week);
    refreshTab(m_btnMonth, m_view == CalendarView::Month);
}

void CalendarPage::setView(CalendarView v) {
    m_view = v;
    switchViewWidget();
    refresh();
    if (v != CalendarView::Month) {
        auto *grid = (v == CalendarView::Week) ? m_weekView : m_dayView;
        grid->scrollToHour(7);
    }
    applyTheme();
}

void CalendarPage::switchViewWidget() {
    switch (m_view) {
        case CalendarView::Month: m_stack->setCurrentWidget(m_monthView); break;
        case CalendarView::Week:  m_stack->setCurrentWidget(m_weekView);  break;
        case CalendarView::Day:   m_stack->setCurrentWidget(m_dayView);   break;
    }
}

void CalendarPage::goToday() {
    m_currentDate = QDate::currentDate();
    refresh();
}

void CalendarPage::goNext() {
    switch (m_view) {
        case CalendarView::Day:   m_currentDate = m_currentDate.addDays(1); break;
        case CalendarView::Week:  m_currentDate = m_currentDate.addDays(7); break;
        case CalendarView::Month: m_currentDate = m_currentDate.addMonths(1); break;
    }
    refresh();
}

void CalendarPage::goPrev() {
    switch (m_view) {
        case CalendarView::Day:   m_currentDate = m_currentDate.addDays(-1); break;
        case CalendarView::Week:  m_currentDate = m_currentDate.addDays(-7); break;
        case CalendarView::Month: m_currentDate = m_currentDate.addMonths(-1); break;
    }
    refresh();
}

void CalendarPage::refresh() {
    QDate gridStart, gridEnd;
    switch (m_view) {
        case CalendarView::Month: {
            QDate first(m_currentDate.year(), m_currentDate.month(), 1);
            int dow = first.dayOfWeek() % 7;
            gridStart = first.addDays(-dow);
            gridEnd = gridStart.addDays(42);
            break;
        }
        case CalendarView::Week: {
            int dow = m_currentDate.dayOfWeek() % 7;
            gridStart = m_currentDate.addDays(-dow);
            gridEnd = gridStart.addDays(7);
            break;
        }
        case CalendarView::Day: {
            gridStart = m_currentDate;
            gridEnd = m_currentDate.addDays(1);
            break;
        }
    }

    QDateTime startDt(gridStart, QTime(0, 0, 0));
    QDateTime endDt(gridEnd, QTime(23, 59, 59));
    m_events = m_db->getEventsByRange(startDt, endDt);

    m_monthView->setCurrentDate(m_currentDate);
    m_monthView->setEvents(m_events);
    m_weekView->setCurrentDate(m_currentDate);
    m_weekView->setEvents(m_events);
    m_dayView->setCurrentDate(m_currentDate);
    m_dayView->setEvents(m_events);

    updateHeader();
}

void CalendarPage::updateHeader() {
    QString title;
    int y = m_currentDate.year();
    int mo = m_currentDate.month();
    int d = m_currentDate.day();
    switch (m_view) {
        case CalendarView::Month:
            title = QString("%1年%2月").arg(y).arg(mo);
            break;
        case CalendarView::Week: {
            int dow = m_currentDate.dayOfWeek() % 7;
            QDate ws = m_currentDate.addDays(-dow);
            QDate we = ws.addDays(6);
            if (ws.month() == we.month()) {
                title = QString("%1年%2月 %3日 - %4日")
                    .arg(ws.year()).arg(ws.month()).arg(ws.day()).arg(we.day());
            } else {
                title = QString("%1月%2日 - %3月%4日")
                    .arg(ws.month()).arg(ws.day()).arg(we.month()).arg(we.day());
            }
            break;
        }
        case CalendarView::Day:
            title = QString("%1年%2月%3日").arg(y).arg(mo).arg(d);
            break;
    }
    m_titleLabel->setText(title);
}

void CalendarPage::openCreateDialog(const QDateTime &dt) {
    QDateTime start = dt;
    if (!start.isValid()) {
        QDateTime n = QDateTime::currentDateTime();
        start = QDateTime(n.date(), QTime(qBound(0, n.time().hour() + 1, 22), 0));
    }
    EventDialog dlg(this);
    dlg.setupForCreate(start);
    if (dlg.exec() == QDialog::Accepted) {
        auto e = dlg.result();
        if (!m_db->insertEvent(e)) {
            QMessageBox::warning(this, "错误", "保存失败");
        }
    }
}

void CalendarPage::onEventClicked(const CalendarEvent &event) {
    EventDialog dlg(this);
    dlg.setupForEdit(event);
    connect(&dlg, &EventDialog::requestDelete, this, [this](const QString &id) {
        m_db->deleteEvent(id);
    });
    if (dlg.exec() == QDialog::Accepted) {
        auto e = dlg.result();
        if (!m_db->updateEvent(e)) {
            QMessageBox::warning(this, "错误", "更新失败");
        }
    }
}

void CalendarPage::onTimeSlotClicked(const QDateTime &dt) {
    openCreateDialog(dt);
}

void CalendarPage::onMonthDateClicked(const QDate &d) {
    QDateTime now = QDateTime::currentDateTime();
    QTime t = (d == QDate::currentDate())
        ? QTime(qBound(0, now.time().hour() + 1, 22), 0)
        : QTime(9, 0);
    openCreateDialog(QDateTime(d, t));
}

void CalendarPage::onMonthOverflowClicked(const QDate &d, const QList<CalendarEvent> &events) {
    QDialog dlg(this);
    dlg.setWindowTitle(QString("%1 — %2 个日程").arg(d.toString("yyyy-MM-dd")).arg(events.size()));
    dlg.setMinimumSize(380, 480);
    dlg.setStyleSheet(Theme::instance().globalStylesheet());
    auto *layout = new QVBoxLayout(&dlg);

    auto *list = new QListWidget(&dlg);
    auto pal = Theme::instance().palette();
    for (const auto &e : events) {
        auto *item = new QListWidgetItem();
        QString time = e.allDay ? "全天" : e.startDate.toString("HH:mm");
        item->setText(QString("  %1   %2").arg(time, e.title));
        QColor c = pal[e.color].text;
        item->setForeground(c);
        item->setData(Qt::UserRole, e.id);
        item->setSizeHint(QSize(0, 38));
        list->addItem(item);
    }
    layout->addWidget(list);

    connect(list, &QListWidget::itemDoubleClicked, this, [this, &dlg, events](QListWidgetItem *item) {
        QString id = item->data(Qt::UserRole).toString();
        for (const auto &e : events) {
            if (e.id == id) {
                dlg.accept();
                onEventClicked(e);
                return;
            }
        }
    });

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(bb);

    dlg.exec();
}

void CalendarPage::onParseClicked() {
    QString text = m_parseInput->text().trimmed();
    if (text.isEmpty()) return;
    if (!m_ai->hasApiKey()) {
        QMessageBox::information(this, "提示",
            "请先在「设置」页填写 DeepSeek API Key。");
        return;
    }
    m_parseStatus->setText("解析中…");
    m_parseButton->setEnabled(false);
    m_ai->parseSchedule(text);
}

void CalendarPage::onParseFinished(const QList<ScheduleSuggestion> &items) {
    m_parseButton->setEnabled(true);
    if (items.isEmpty()) {
        m_parseStatus->setText("未识别到日程");
        return;
    }

    // 弹窗确认列表
    QDialog dlg(this);
    dlg.setWindowTitle("AI 识别结果");
    dlg.setMinimumSize(520, 480);
    dlg.setStyleSheet(Theme::instance().globalStylesheet());
    auto *layout = new QVBoxLayout(&dlg);

    auto *header = new QLabel(QString("识别到 %1 条日程，请勾选要添加的：").arg(items.size()));
    header->setProperty("class", "subtitle");
    layout->addWidget(header);

    auto *list = new QListWidget(&dlg);
    list->setSelectionMode(QAbstractItemView::NoSelection);
    auto pal = Theme::instance().palette();
    for (int i = 0; i < items.size(); ++i) {
        const auto &s = items[i];
        auto *item = new QListWidgetItem();
        QString time = s.allDay
            ? "全天"
            : (s.startDate.toString("MM-dd HH:mm")
               + " — " + s.endDate.toString("HH:mm"));
        item->setText(QString("  %1\n  %2 · %3 · %4")
                          .arg(s.title, time, categoryLabel(s.category),
                               priorityLabel(s.priority)));
        item->setCheckState(Qt::Checked);
        QColor c = pal[s.color].text;
        item->setForeground(c);
        item->setSizeHint(QSize(0, 56));
        list->addItem(item);
    }
    layout->addWidget(list, 1);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    bb->button(QDialogButtonBox::Save)->setText("添加到日历");
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    layout->addWidget(bb);

    if (dlg.exec() != QDialog::Accepted) {
        m_parseStatus->setText("已取消");
        return;
    }

    int added = 0;
    for (int i = 0; i < items.size(); ++i) {
        if (list->item(i)->checkState() != Qt::Checked) continue;
        const auto &s = items[i];
        CalendarEvent e;
        e.id = Database::generateId();
        e.title = s.title;
        e.description = s.description;
        e.startDate = s.startDate.isValid() ? s.startDate : QDateTime::currentDateTime();
        e.endDate = s.endDate.isValid() ? s.endDate : e.startDate.addSecs(60 * s.durationMinutes);
        e.allDay = s.allDay;
        e.color = s.color;
        e.category = s.category;
        e.priority = s.priority;
        e.source = EventSource::AiParse;
        e.createdAt = QDateTime::currentDateTime();
        e.updatedAt = e.createdAt;
        if (m_db->insertEvent(e)) added++;
    }
    m_parseStatus->setText(QString("已添加 %1 条").arg(added));
    m_parseInput->clear();
}

void CalendarPage::onParseError(const QString &msg) {
    m_parseButton->setEnabled(true);
    m_parseStatus->setText("解析失败");
    QMessageBox::warning(this, "AI 解析失败", msg);
}

} // namespace timeplan
