#include "EventDialog.h"
#include "Theme.h"
#include "../core/Database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>

namespace timemaster {

EventDialog::EventDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("事件");
    setModal(true);
    setMinimumSize(560, 660);
    buildUi();
    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, &EventDialog::applyTheme);
}

void EventDialog::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(18);

    auto *titleLabel = new QLabel("事件详情");
    titleLabel->setProperty("class", "title");
    root->addWidget(titleLabel);

    // 标题
    {
        auto *box = new QVBoxLayout();
        auto *l = new QLabel("标题");
        l->setProperty("class", "caption");
        box->addWidget(l);
        m_titleEdit = new QLineEdit();
        m_titleEdit->setPlaceholderText("如：项目评审会");
        m_titleEdit->setMinimumHeight(36);
        box->addWidget(m_titleEdit);
        root->addLayout(box);
    }

    // 时间行
    {
        auto *grid = new QGridLayout();
        grid->setHorizontalSpacing(12);
        grid->setVerticalSpacing(8);
        auto *lStart = new QLabel("开始");
        lStart->setProperty("class", "caption");
        auto *lEnd = new QLabel("结束");
        lEnd->setProperty("class", "caption");
        grid->addWidget(lStart, 0, 0);
        grid->addWidget(lEnd, 0, 1);

        m_startEdit = new QDateTimeEdit();
        m_startEdit->setCalendarPopup(true);
        m_startEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
        m_startEdit->setMinimumHeight(36);
        m_endEdit = new QDateTimeEdit();
        m_endEdit->setCalendarPopup(true);
        m_endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
        m_endEdit->setMinimumHeight(36);
        grid->addWidget(m_startEdit, 1, 0);
        grid->addWidget(m_endEdit, 1, 1);

        connect(m_startEdit, &QDateTimeEdit::dateTimeChanged,
                this, &EventDialog::syncEndDateMin);

        m_allDayCheck = new QCheckBox("全天事件");
        grid->addWidget(m_allDayCheck, 2, 0, 1, 2);
        connect(m_allDayCheck, &QCheckBox::toggled, this, &EventDialog::onAllDayToggled);

        root->addLayout(grid);
    }

    // 类别
    {
        auto *l = new QLabel("类别");
        l->setProperty("class", "caption");
        root->addWidget(l);

        auto *row = new QHBoxLayout();
        row->setSpacing(6);
        auto cats = allCategories();
        for (int i = 0; i < cats.size(); ++i) {
            auto *btn = new QPushButton(categoryLabel(cats[i]));
            btn->setCheckable(true);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setMinimumHeight(30);
            connect(btn, &QPushButton::clicked, this, [this, i] { onCategoryClicked(i); });
            m_categoryButtons.append(btn);
            row->addWidget(btn);
        }
        row->addStretch();
        root->addLayout(row);
    }

    // 优先级
    {
        auto *l = new QLabel("优先级");
        l->setProperty("class", "caption");
        root->addWidget(l);

        auto *row = new QHBoxLayout();
        row->setSpacing(6);
        auto prios = allPriorities();
        for (int i = 0; i < prios.size(); ++i) {
            auto *btn = new QPushButton(priorityLabel(prios[i]));
            btn->setCheckable(true);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setMinimumHeight(30);
            btn->setMinimumWidth(80);
            connect(btn, &QPushButton::clicked, this, [this, i] { onPriorityClicked(i); });
            m_priorityButtons.append(btn);
            row->addWidget(btn);
        }
        row->addStretch();
        root->addLayout(row);
    }

    // 颜色
    {
        auto *l = new QLabel("颜色");
        l->setProperty("class", "caption");
        root->addWidget(l);

        auto *row = new QHBoxLayout();
        row->setSpacing(8);
        auto colors = allColors();
        for (int i = 0; i < colors.size(); ++i) {
            auto *btn = new QPushButton();
            btn->setCheckable(true);
            btn->setFixedSize(28, 28);
            btn->setCursor(Qt::PointingHandCursor);
            connect(btn, &QPushButton::clicked, this, [this, i] { onColorClicked(i); });
            m_colorButtons.append(btn);
            row->addWidget(btn);
        }
        row->addStretch();
        root->addLayout(row);
    }

    // 地点 + 提醒
    {
        auto *grid = new QGridLayout();
        grid->setHorizontalSpacing(12);
        grid->setVerticalSpacing(8);

        auto *lLoc = new QLabel("地点（可选）");
        lLoc->setProperty("class", "caption");
        m_locationEdit = new QLineEdit();
        m_locationEdit->setPlaceholderText("如：会议室 A");
        m_locationEdit->setMinimumHeight(36);
        grid->addWidget(lLoc, 0, 0);
        grid->addWidget(m_locationEdit, 1, 0);

        auto *lRem = new QLabel("提前提醒（分钟）");
        lRem->setProperty("class", "caption");
        m_reminderSpin = new QSpinBox();
        m_reminderSpin->setRange(0, 1440);
        m_reminderSpin->setSingleStep(5);
        m_reminderSpin->setValue(15);
        m_reminderSpin->setMinimumHeight(36);
        grid->addWidget(lRem, 0, 1);
        grid->addWidget(m_reminderSpin, 1, 1);

        root->addLayout(grid);
    }

    // 备注
    {
        auto *l = new QLabel("备注（可选）");
        l->setProperty("class", "caption");
        root->addWidget(l);
        m_descriptionEdit = new QPlainTextEdit();
        m_descriptionEdit->setPlaceholderText("补充说明…");
        m_descriptionEdit->setMaximumHeight(80);
        root->addWidget(m_descriptionEdit);
    }

    root->addStretch(1);

    {
        auto *row = new QHBoxLayout();
        m_deleteButton = new QPushButton("删除");
        m_deleteButton->setProperty("class", "ghost");
        m_deleteButton->setMinimumHeight(36);
        m_deleteButton->setVisible(false);
        connect(m_deleteButton, &QPushButton::clicked, this, &EventDialog::onDeleteClicked);
        row->addWidget(m_deleteButton);
        row->addStretch();

        m_cancelButton = new QPushButton("取消");
        m_cancelButton->setMinimumSize(86, 36);
        connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        row->addWidget(m_cancelButton);

        m_submitButton = new QPushButton("保存");
        m_submitButton->setProperty("class", "primary");
        m_submitButton->setMinimumSize(86, 36);
        m_submitButton->setDefault(true);
        connect(m_submitButton, &QPushButton::clicked, this, &EventDialog::onSubmit);
        row->addWidget(m_submitButton);

        root->addLayout(row);
    }
}

void EventDialog::applyTheme() {
    setStyleSheet(Theme::instance().globalStylesheet());
    refreshCategoryButtons();
    refreshPriorityButtons();
    refreshColorButtons();
}

void EventDialog::setupForCreate(const QDateTime &defaultStart) {
    m_isEditing = false;
    m_eventId.clear();
    m_aiBatchId.clear();
    m_source = EventSource::Manual;
    m_createdAt = QDateTime::currentDateTime();

    m_titleEdit->clear();
    m_locationEdit->clear();
    m_descriptionEdit->clear();

    QDateTime start = defaultStart;
    if (!start.isValid()) start = QDateTime::currentDateTime();
    if (start.time() == QTime(0, 0, 0)) {
        start.setTime(QTime(qBound(0, QDateTime::currentDateTime().time().hour() + 1, 23), 0));
    }
    m_startEdit->setDateTime(start);
    m_endEdit->setDateTime(start.addSecs(3600));

    m_allDayCheck->setChecked(false);
    m_reminderSpin->setValue(15);

    m_selectedCategory = EventCategory::Work;
    m_selectedPriority = EventPriority::Normal;
    m_selectedColor = EventColor::Blue;
    refreshCategoryButtons();
    refreshPriorityButtons();
    refreshColorButtons();

    m_deleteButton->setVisible(false);
    m_titleEdit->setFocus();
}

void EventDialog::setupForEdit(const CalendarEvent &event) {
    m_isEditing = true;
    m_eventId = event.id;
    m_aiBatchId = event.aiBatchId;
    m_source = event.source;
    m_createdAt = event.createdAt;

    m_titleEdit->setText(event.title);
    m_locationEdit->setText(event.location);
    m_descriptionEdit->setPlainText(event.description);
    m_startEdit->setDateTime(event.startDate);
    m_endEdit->setDateTime(event.endDate);
    m_allDayCheck->setChecked(event.allDay);
    m_reminderSpin->setValue(event.reminder);

    m_selectedCategory = event.category;
    m_selectedPriority = event.priority;
    m_selectedColor = event.color;
    refreshCategoryButtons();
    refreshPriorityButtons();
    refreshColorButtons();

    onAllDayToggled(event.allDay);

    m_deleteButton->setVisible(true);
    m_titleEdit->setFocus();
}

void EventDialog::onAllDayToggled(bool checked) {
    if (checked) {
        m_startEdit->setDisplayFormat("yyyy-MM-dd");
        m_endEdit->setDisplayFormat("yyyy-MM-dd");
    } else {
        m_startEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
        m_endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    }
}

void EventDialog::onCategoryClicked(int idx) {
    auto cats = allCategories();
    if (idx < 0 || idx >= cats.size()) return;
    m_selectedCategory = cats[idx];
    m_selectedColor = categoryDefaultColor(m_selectedCategory);
    refreshCategoryButtons();
    refreshColorButtons();
}

void EventDialog::onPriorityClicked(int idx) {
    auto prios = allPriorities();
    if (idx < 0 || idx >= prios.size()) return;
    m_selectedPriority = prios[idx];
    refreshPriorityButtons();
}

void EventDialog::onColorClicked(int idx) {
    auto cols = allColors();
    if (idx < 0 || idx >= cols.size()) return;
    m_selectedColor = cols[idx];
    refreshColorButtons();
}

void EventDialog::refreshCategoryButtons() {
    auto cats = allCategories();
    auto &theme = Theme::instance();
    QString brandLight = QString("rgba(%1,%2,%3,30)")
                            .arg(theme.brand().red()).arg(theme.brand().green()).arg(theme.brand().blue());
    for (int i = 0; i < m_categoryButtons.size(); ++i) {
        auto *btn = m_categoryButtons[i];
        bool active = (cats[i] == m_selectedCategory);
        btn->setChecked(active);
        if (active) {
            btn->setStyleSheet(QString(
                "QPushButton{background:%1;color:%2;border:1px solid %3;border-radius:15px;"
                "padding:4px 14px;font-weight:600;}"
            ).arg(brandLight, theme.brand().name(), theme.brand().name()));
        } else {
            btn->setStyleSheet(QString(
                "QPushButton{background:transparent;color:%1;border:1px solid %2;"
                "border-radius:15px;padding:4px 14px;}"
                "QPushButton:hover{background:%3;}"
            ).arg(theme.textSecondary().name(),
                  theme.stroke().name(),
                  theme.bgHover().name()));
        }
    }
}

void EventDialog::refreshPriorityButtons() {
    auto prios = allPriorities();
    QStringList tints = {"#B8453E", "#C28E3D", "#6E6760"};  // 紧急(砖红) · 重要(琥珀金) · 普通(暖灰)
    for (int i = 0; i < m_priorityButtons.size(); ++i) {
        auto *btn = m_priorityButtons[i];
        bool active = (prios[i] == m_selectedPriority);
        btn->setChecked(active);
        if (active) {
            btn->setStyleSheet(QString(
                "QPushButton{background:%1;color:white;border:none;border-radius:15px;"
                "padding:4px 18px;font-weight:600;}"
            ).arg(tints[i]));
        } else {
            btn->setStyleSheet(QString(
                "QPushButton{background:transparent;color:%1;border:1px solid %2;"
                "border-radius:15px;padding:4px 18px;}"
                "QPushButton:hover{background:%3;}"
            ).arg(Theme::instance().textSecondary().name(),
                  Theme::instance().stroke().name(),
                  Theme::instance().bgHover().name()));
        }
    }
}

void EventDialog::refreshColorButtons() {
    auto cols = allColors();
    auto pal = Theme::instance().palette();
    for (int i = 0; i < m_colorButtons.size(); ++i) {
        auto *btn = m_colorButtons[i];
        bool active = (cols[i] == m_selectedColor);
        QColor c = pal[cols[i]].text;
        if (active) {
            btn->setStyleSheet(QString(
                "QPushButton{background:%1;border:3px solid %2;border-radius:14px;}"
            ).arg(c.name(), Theme::instance().textPrimary().name()));
        } else {
            btn->setStyleSheet(QString(
                "QPushButton{background:%1;border:1px solid %2;border-radius:14px;}"
                "QPushButton:hover{border:2px solid %3;}"
            ).arg(c.name(),
                  Theme::instance().stroke().name(),
                  Theme::instance().textSecondary().name()));
        }
    }
}

void EventDialog::syncEndDateMin() {
    if (m_endEdit->dateTime() < m_startEdit->dateTime()) {
        m_endEdit->setDateTime(m_startEdit->dateTime().addSecs(3600));
    }
}

void EventDialog::onSubmit() {
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::information(this, "提示", "请填写事件标题");
        m_titleEdit->setFocus();
        return;
    }
    if (m_endEdit->dateTime() <= m_startEdit->dateTime()) {
        QMessageBox::information(this, "提示", "结束时间必须晚于开始时间");
        return;
    }
    accept();
}

void EventDialog::onDeleteClicked() {
    auto ret = QMessageBox::question(this, "确认删除",
        "确定要删除「" + m_titleEdit->text().trimmed() + "」吗？此操作无法撤销。",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        emit requestDelete(m_eventId);
        reject();
    }
}

CalendarEvent EventDialog::result() const {
    CalendarEvent e;
    e.id = m_isEditing ? m_eventId : Database::generateId();
    e.title = m_titleEdit->text().trimmed();
    e.description = m_descriptionEdit->toPlainText();
    e.startDate = m_startEdit->dateTime();
    e.endDate = m_endEdit->dateTime();
    e.allDay = m_allDayCheck->isChecked();
    e.color = m_selectedColor;
    e.category = m_selectedCategory;
    e.location = m_locationEdit->text().trimmed();
    e.reminder = m_reminderSpin->value();
    e.priority = m_selectedPriority;
    e.source = m_isEditing ? m_source : EventSource::Manual;
    e.aiBatchId = m_aiBatchId;
    e.createdAt = m_isEditing ? m_createdAt : QDateTime::currentDateTime();
    e.updatedAt = QDateTime::currentDateTime();
    return e;
}

} // namespace timemaster
