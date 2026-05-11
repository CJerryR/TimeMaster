#include "CalendarPage.h"
#include "Theme.h"
#include "MonthView.h"
#include "TimeGridView.h"
#include "EventDialog.h"
#include "AiHistoryDialog.h"
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

namespace timemaster {

CalendarPage::CalendarPage(Database *db, DeepSeekClient *ai, QWidget *parent)
    : QWidget(parent), m_db(db), m_ai(ai)
{
    setAttribute(Qt::WA_StyledBackground, false);
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
    root->setContentsMargins(20, 18, 20, 20);
    root->setSpacing(14);

    // ============ AI 解析输入条（卡片样式） ============
    auto *parseCard = new QWidget;
    parseCard->setObjectName("AiInputCard");
    auto *parseLayout = new QHBoxLayout(parseCard);
    parseLayout->setContentsMargins(14, 10, 10, 10);
    parseLayout->setSpacing(10);

    auto *iconLabel = new QLabel("✨");
    QFont iconFont; iconFont.setPointSize(16);
    iconLabel->setFont(iconFont);
    parseLayout->addWidget(iconLabel);

    m_parseInput = new QLineEdit;
    m_parseInput->setObjectName("AiInputEdit");
    m_parseInput->setPlaceholderText("用自然语言告诉 AI 你的日程，比如：明天下午 3 点项目评审、周三上午健身一小时");
    m_parseInput->setMinimumHeight(40);
    parseLayout->addWidget(m_parseInput, 1);

    m_historyButton = new QPushButton("🕐 导入历史");
    m_historyButton->setObjectName("HistoryBtn");
    m_historyButton->setCursor(Qt::PointingHandCursor);
    m_historyButton->setMinimumHeight(40);
    m_historyButton->setToolTip("查看并撤销 AI 导入的日程");
    parseLayout->addWidget(m_historyButton);

    m_parseButton = new QPushButton("AI 解析");
    m_parseButton->setProperty("class", "primary");
    m_parseButton->setMinimumSize(96, 40);
    m_parseButton->setCursor(Qt::PointingHandCursor);
    parseLayout->addWidget(m_parseButton);

    m_parseStatus = new QLabel;
    m_parseStatus->setObjectName("ParseStatusLabel");
    m_parseStatus->setMinimumWidth(80);
    parseLayout->addWidget(m_parseStatus);

    root->addWidget(parseCard);

    connect(m_parseButton, &QPushButton::clicked, this, &CalendarPage::onParseClicked);
    connect(m_parseInput, &QLineEdit::returnPressed, this, &CalendarPage::onParseClicked);
    connect(m_historyButton, &QPushButton::clicked, this, &CalendarPage::onHistoryClicked);

    // ============ 头部（标题 + 导航 + 视图切换） ============
    auto *headerWidget = new QWidget;
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(2, 0, 2, 0);
    headerLayout->setSpacing(10);

    auto *btnAdd = new QPushButton("+ 新建日程");
    btnAdd->setProperty("class", "primary");
    btnAdd->setMinimumSize(110, 34);
    btnAdd->setCursor(Qt::PointingHandCursor);
    connect(btnAdd, &QPushButton::clicked, this, [this] { openCreateDialog(); });
    headerLayout->addWidget(btnAdd);

    headerLayout->addSpacing(6);

    auto *btnPrev = new QPushButton("‹");
    btnPrev->setObjectName("NavArrow");
    btnPrev->setFixedSize(34, 34);
    btnPrev->setCursor(Qt::PointingHandCursor);
    connect(btnPrev, &QPushButton::clicked, this, &CalendarPage::goPrev);
    headerLayout->addWidget(btnPrev);

    auto *btnNext = new QPushButton("›");
    btnNext->setObjectName("NavArrow");
    btnNext->setFixedSize(34, 34);
    btnNext->setCursor(Qt::PointingHandCursor);
    connect(btnNext, &QPushButton::clicked, this, &CalendarPage::goNext);
    headerLayout->addWidget(btnNext);

    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("CalendarTitle");
    QFont tf; tf.setPointSize(19); tf.setWeight(QFont::Bold);
    m_titleLabel->setFont(tf);
    m_titleLabel->setMinimumWidth(220);
    headerLayout->addWidget(m_titleLabel);

    auto *btnToday = new QPushButton("今天");
    btnToday->setObjectName("TodayBtn");
    btnToday->setMinimumSize(68, 32);
    btnToday->setCursor(Qt::PointingHandCursor);
    connect(btnToday, &QPushButton::clicked, this, &CalendarPage::goToday);
    headerLayout->addWidget(btnToday);

    headerLayout->addStretch();

    // 视图切换分段（用一个容器实现胶囊外观）
    auto *segWrap = new QWidget;
    segWrap->setObjectName("ViewSegment");
    auto *segLayout = new QHBoxLayout(segWrap);
    segLayout->setContentsMargins(4, 4, 4, 4);
    segLayout->setSpacing(2);

    m_btnDay = new QPushButton("日");
    m_btnWeek = new QPushButton("周");
    m_btnMonth = new QPushButton("月");
    for (auto *b : {m_btnDay, m_btnWeek, m_btnMonth}) {
        b->setObjectName("ViewSegBtn");
        b->setCheckable(true);
        b->setMinimumSize(46, 28);
        b->setCursor(Qt::PointingHandCursor);
        segLayout->addWidget(b);
    }
    headerLayout->addWidget(segWrap);

    connect(m_btnDay,   &QPushButton::clicked, this, [this]{ setView(CalendarView::Day); });
    connect(m_btnWeek,  &QPushButton::clicked, this, [this]{ setView(CalendarView::Week); });
    connect(m_btnMonth, &QPushButton::clicked, this, [this]{ setView(CalendarView::Month); });

    root->addWidget(headerWidget);

    // ============ 视图区（卡片容器） ============
    auto *viewCard = new QWidget;
    viewCard->setObjectName("ViewCard");
    auto *viewLayout = new QVBoxLayout(viewCard);
    viewLayout->setContentsMargins(0, 0, 0, 0);

    m_stack = new QStackedWidget(this);
    m_stack->setStyleSheet("QStackedWidget{background:transparent;}");
    m_monthView = new MonthView(this);
    m_weekView = new TimeGridView(TimeGridView::WeekMode, this);
    m_dayView = new TimeGridView(TimeGridView::DayMode, this);
    m_stack->addWidget(m_monthView);
    m_stack->addWidget(m_weekView);
    m_stack->addWidget(m_dayView);

    viewLayout->addWidget(m_stack);
    root->addWidget(viewCard, 1);

    connect(m_monthView, &MonthView::dateClicked, this, &CalendarPage::onMonthDateClicked);
    connect(m_monthView, &MonthView::eventClicked, this, &CalendarPage::onEventClicked);
    connect(m_monthView, &MonthView::overflowClicked, this, &CalendarPage::onMonthOverflowClicked);

    connect(m_weekView, &TimeGridView::timeSlotClicked, this, &CalendarPage::onTimeSlotClicked);
    connect(m_weekView, &TimeGridView::eventClicked, this, &CalendarPage::onEventClicked);
    connect(m_dayView, &TimeGridView::timeSlotClicked, this, &CalendarPage::onTimeSlotClicked);
    connect(m_dayView, &TimeGridView::eventClicked, this, &CalendarPage::onEventClicked);

    // ============ 快捷键 ============
    auto bind = [this](const char *key, std::function<void()> fn) {
        auto *sc = new QShortcut(QKeySequence(key), this);
        connect(sc, &QShortcut::activated, this, fn);
    };
    bind("Ctrl+N", [this]{ openCreateDialog(); });
    bind("Ctrl+Z", [this]{ onHistoryClicked(); });  // 撤销 = 打开历史面板
    bind("T",      [this]{ goToday(); });
    bind("Left",   [this]{ goPrev(); });
    bind("Right",  [this]{ goNext(); });
    bind("1",      [this]{ setView(CalendarView::Day); });
    bind("2",      [this]{ setView(CalendarView::Week); });
    bind("3",      [this]{ setView(CalendarView::Month); });

    setView(CalendarView::Month);
}

void CalendarPage::applyTheme() {
    auto &t = Theme::instance();

    QString brand = t.brand().name();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();
    QString placeholder = t.textPlaceholder().name();
    QString strokeR = t.strokeRgba();
    QString cardBg = t.cardBgRgba();
    QString componentBg = t.componentBgRgba();
    QString hoverBg = t.cardBgHoverRgba();

    setStyleSheet(t.globalStylesheet() + QString(R"(
        QWidget#AiInputCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 14px;
        }
        QLineEdit#AiInputEdit {
            background-color: transparent;
            border: none;
            font-size: 14px;
            padding: 4px 6px;
            color: %3;
        }
        QLineEdit#AiInputEdit:focus { border: none; }
        QPushButton#HistoryBtn {
            background-color: %4;
            border: 1px solid %2;
            border-radius: 10px;
            padding: 0 14px;
            color: %5;
            font-weight: 500;
        }
        QPushButton#HistoryBtn:hover {
            background-color: %6;
            color: %3;
        }
        QLabel#ParseStatusLabel {
            color: %5;
            font-size: 12px;
        }

        QLabel#CalendarTitle {
            color: %3;
            background: transparent;
        }
        QPushButton#NavArrow {
            background-color: transparent;
            border: 1px solid %2;
            border-radius: 9px;
            color: %5;
            font-size: 18px;
            font-weight: 500;
            padding-bottom: 2px;
        }
        QPushButton#NavArrow:hover {
            background-color: %6;
            color: %3;
        }
        QPushButton#TodayBtn {
            background-color: transparent;
            border: 1px solid %2;
            border-radius: 9px;
            color: %5;
            font-weight: 500;
        }
        QPushButton#TodayBtn:hover {
            background-color: %6;
            color: %3;
        }

        QWidget#ViewSegment {
            background-color: %4;
            border: 1px solid %2;
            border-radius: 12px;
        }
        QPushButton#ViewSegBtn {
            background-color: transparent;
            border: none;
            border-radius: 8px;
            color: %5;
            font-weight: 500;
            padding: 0 6px;
        }
        QPushButton#ViewSegBtn:hover {
            background-color: rgba(120,120,140,0.10);
            color: %3;
        }
        QPushButton#ViewSegBtn:checked {
            background-color: %7;
            color: white;
            font-weight: 600;
        }

        QWidget#ViewCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 14px;
        }
    )")
    /*1*/.arg(cardBg)
    /*2*/.arg(strokeR)
    /*3*/.arg(textPrim)
    /*4*/.arg(componentBg)
    /*5*/.arg(textSec)
    /*6*/.arg(hoverBg)
    /*7*/.arg(brand));
}

void CalendarPage::setView(CalendarView v) {
    m_view = v;
    switchViewWidget();
    m_btnDay->setChecked(v == CalendarView::Day);
    m_btnWeek->setChecked(v == CalendarView::Week);
    m_btnMonth->setChecked(v == CalendarView::Month);
    refresh();
    if (v != CalendarView::Month) {
        auto *grid = (v == CalendarView::Week) ? m_weekView : m_dayView;
        grid->scrollToHour(7);
    }
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
            title = QString("%1 年 %2 月").arg(y).arg(mo);
            break;
        case CalendarView::Week: {
            int dow = m_currentDate.dayOfWeek() % 7;
            QDate ws = m_currentDate.addDays(-dow);
            QDate we = ws.addDays(6);
            if (ws.month() == we.month()) {
                title = QString("%1 年 %2 月 %3 — %4 日")
                    .arg(ws.year()).arg(ws.month()).arg(ws.day()).arg(we.day());
            } else {
                title = QString("%1月%2日 — %3月%4日")
                    .arg(ws.month()).arg(ws.day()).arg(we.month()).arg(we.day());
            }
            break;
        }
        case CalendarView::Day:
            title = QString("%1 年 %2 月 %3 日").arg(y).arg(mo).arg(d);
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
    dlg.setMinimumSize(400, 480);
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
    m_pendingParseText = text;
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

    QDialog dlg(this);
    dlg.setWindowTitle("AI 识别结果");
    dlg.setMinimumSize(560, 500);
    dlg.setStyleSheet(Theme::instance().globalStylesheet());
    auto *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(22, 20, 22, 18);

    auto *header = new QLabel(QString("✨ 识别到 %1 条日程，请确认要导入的：").arg(items.size()));
    header->setProperty("class", "subtitle");
    layout->addWidget(header);

    auto *tip = new QLabel("导入后如有错误，可通过顶部「🕐 导入历史」整批撤销或单条删除。");
    tip->setProperty("class", "caption");
    tip->setWordWrap(true);
    layout->addWidget(tip);
    layout->addSpacing(6);

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
    bb->button(QDialogButtonBox::Save)->setText("导入");
    bb->button(QDialogButtonBox::Save)->setProperty("class", "primary");
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    layout->addWidget(bb);

    if (dlg.exec() != QDialog::Accepted) {
        m_parseStatus->setText("已取消");
        return;
    }

    // 创建一个 batch
    QString batchId = m_db->createAiBatch(m_pendingParseText, "parse");

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
        e.aiBatchId = batchId;
        e.createdAt = QDateTime::currentDateTime();
        e.updatedAt = e.createdAt;
        if (m_db->insertEvent(e)) added++;
    }
    m_parseStatus->setText(QString("✓ 已导入 %1 条").arg(added));
    m_parseInput->clear();
    m_pendingParseText.clear();
}

void CalendarPage::onParseError(const QString &msg) {
    m_parseButton->setEnabled(true);
    m_parseStatus->setText("解析失败");
    QMessageBox::warning(this, "AI 解析失败", msg);
}

void CalendarPage::onHistoryClicked() {
    AiHistoryDialog dlg(m_db, this);
    dlg.exec();
}

} // namespace timemaster
