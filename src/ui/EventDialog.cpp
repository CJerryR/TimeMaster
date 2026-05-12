#include "EventDialog.h"
#include "Theme.h"
#include "widgets/FlowLayout.h"
#include "../core/Database.h"
#include "../core/I18n.h"

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
    setObjectName("EventDialog");          // V4.3 #2 — QSS 钩子让对话框背景跟主题
    setWindowTitle(I18n::t("event.title"));
    setModal(true);
    // V4.3 #1 — bumped from 560x660. 字号长大后原来的最小尺寸把控件挤在一起，
    // 在某些 DPI 下出现 caption 与 input 文本重叠。
    setMinimumSize(600, 720);
    buildUi();
    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, &EventDialog::applyTheme);
}

void EventDialog::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(16);                  // V4.3 #1 — 从 18 略调到 16，整体均衡

    m_titleLabel = new QLabel(I18n::t("event.title"));
    m_titleLabel->setObjectName("EventDialogTitle");
    m_titleLabel->setProperty("class", "title");
    root->addWidget(m_titleLabel);

    // 标题
    {
        auto *box = new QVBoxLayout();
        box->setSpacing(6);                // V4.3 #1 — caption 与 input 之间留 6
        auto *l = new QLabel(I18n::t("event.field.title"));
        l->setProperty("class", "caption");
        l->setMinimumHeight(22);          // V4.3 #1 — caption 不再被 descender 压塌
        box->addWidget(l);
        m_titleEdit = new QLineEdit();
        m_titleEdit->setPlaceholderText(I18n::t("event.title_ph"));
        m_titleEdit->setMinimumHeight(40);  // V4.3 #1 — 36 → 40
        box->addWidget(m_titleEdit);
        root->addLayout(box);
    }

    // 时间行
    {
        auto *grid = new QGridLayout();
        grid->setHorizontalSpacing(14);
        grid->setVerticalSpacing(12);     // V4.3 #1 — 8 → 12，避免行间重叠
        auto *lStart = new QLabel(I18n::t("event.start"));
        lStart->setProperty("class", "caption");
        lStart->setMinimumHeight(22);
        auto *lEnd = new QLabel(I18n::t("event.end"));
        lEnd->setProperty("class", "caption");
        lEnd->setMinimumHeight(22);
        grid->addWidget(lStart, 0, 0);
        grid->addWidget(lEnd, 0, 1);

        m_startEdit = new QDateTimeEdit();
        m_startEdit->setCalendarPopup(true);
        m_startEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
        m_startEdit->setMinimumHeight(40);
        m_endEdit = new QDateTimeEdit();
        m_endEdit->setCalendarPopup(true);
        m_endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
        m_endEdit->setMinimumHeight(40);
        grid->addWidget(m_startEdit, 1, 0);
        grid->addWidget(m_endEdit, 1, 1);

        connect(m_startEdit, &QDateTimeEdit::dateTimeChanged,
                this, &EventDialog::syncEndDateMin);

        m_allDayCheck = new QCheckBox(I18n::t("event.all_day"));
        grid->addWidget(m_allDayCheck, 2, 0, 1, 2);
        connect(m_allDayCheck, &QCheckBox::toggled, this, &EventDialog::onAllDayToggled);

        root->addLayout(grid);
    }

    // 类别 (V4.3 #4 — FlowLayout 换行)
    {
        auto *box = new QVBoxLayout();
        box->setSpacing(6);
        auto *l = new QLabel(I18n::t("event.category"));
        l->setProperty("class", "caption");
        l->setMinimumHeight(22);
        box->addWidget(l);

        // FlowLayout 自动换行；不再用 QHBoxLayout 强排一行。
        auto *flowHost = new QWidget;
        flowHost->setObjectName("EventDialogCategoryHost");
        auto *flow = new FlowLayout(flowHost, 0, 6, 6);
        auto cats = allCategories();
        for (int i = 0; i < cats.size(); ++i) {
            auto *btn = new QPushButton(categoryLabel(cats[i]));
            btn->setCheckable(true);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setMinimumHeight(32);
            btn->setFocusPolicy(Qt::NoFocus);
            connect(btn, &QPushButton::clicked, this, [this, i] { onCategoryClicked(i); });
            m_categoryButtons.append(btn);
            flow->addWidget(btn);
        }
        box->addWidget(flowHost);
        root->addLayout(box);
    }

    // 优先级
    {
        auto *box = new QVBoxLayout();
        box->setSpacing(6);
        auto *l = new QLabel(I18n::t("event.priority"));
        l->setProperty("class", "caption");
        l->setMinimumHeight(22);
        box->addWidget(l);

        auto *row = new QHBoxLayout();
        row->setSpacing(6);
        auto prios = allPriorities();
        for (int i = 0; i < prios.size(); ++i) {
            auto *btn = new QPushButton(priorityLabel(prios[i]));
            btn->setCheckable(true);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setMinimumHeight(32);
            btn->setMinimumWidth(86);
            btn->setFocusPolicy(Qt::NoFocus);
            connect(btn, &QPushButton::clicked, this, [this, i] { onPriorityClicked(i); });
            m_priorityButtons.append(btn);
            row->addWidget(btn);
        }
        row->addStretch();
        box->addLayout(row);
        root->addLayout(box);
    }

    // 颜色 (V4.3 #3 — 12 颜色全部可见；FlowLayout 自动换行)
    {
        auto *box = new QVBoxLayout();
        box->setSpacing(6);
        auto *l = new QLabel(I18n::t("event.color"));
        l->setProperty("class", "caption");
        l->setMinimumHeight(22);
        box->addWidget(l);

        auto *flowHost = new QWidget;
        auto *flow = new FlowLayout(flowHost, 0, 8, 8);
        auto colors = allColors();
        for (int i = 0; i < colors.size(); ++i) {
            auto *btn = new QPushButton();
            btn->setCheckable(true);
            btn->setFixedSize(30, 30);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFocusPolicy(Qt::NoFocus);
            connect(btn, &QPushButton::clicked, this, [this, i] { onColorClicked(i); });
            m_colorButtons.append(btn);
            flow->addWidget(btn);
        }
        box->addWidget(flowHost);
        root->addLayout(box);
    }

    // 地点 + 提醒
    {
        auto *grid = new QGridLayout();
        grid->setHorizontalSpacing(14);
        grid->setVerticalSpacing(12);

        auto *lLoc = new QLabel(I18n::t("event.location"));
        lLoc->setProperty("class", "caption");
        lLoc->setMinimumHeight(22);
        m_locationEdit = new QLineEdit();
        m_locationEdit->setPlaceholderText(I18n::t("event.location_ph"));
        m_locationEdit->setMinimumHeight(40);
        grid->addWidget(lLoc, 0, 0);
        grid->addWidget(m_locationEdit, 1, 0);

        auto *lRem = new QLabel(I18n::t("event.reminder"));
        lRem->setProperty("class", "caption");
        lRem->setMinimumHeight(22);
        m_reminderSpin = new QSpinBox();
        m_reminderSpin->setRange(0, 1440);
        m_reminderSpin->setSingleStep(5);
        m_reminderSpin->setValue(15);
        m_reminderSpin->setMinimumHeight(40);
        grid->addWidget(lRem, 0, 1);
        grid->addWidget(m_reminderSpin, 1, 1);

        root->addLayout(grid);
    }

    // 备注
    {
        auto *box = new QVBoxLayout();
        box->setSpacing(6);
        auto *l = new QLabel(I18n::t("event.notes"));
        l->setProperty("class", "caption");
        l->setMinimumHeight(22);
        box->addWidget(l);
        m_descriptionEdit = new QPlainTextEdit();
        m_descriptionEdit->setPlaceholderText(I18n::t("event.notes_ph"));
        m_descriptionEdit->setMaximumHeight(90);
        m_descriptionEdit->setMinimumHeight(64);
        box->addWidget(m_descriptionEdit);
        root->addLayout(box);
    }

    root->addStretch(1);

    {
        auto *row = new QHBoxLayout();
        m_deleteButton = new QPushButton(I18n::t("event.delete"));
        m_deleteButton->setProperty("class", "ghost");
        m_deleteButton->setMinimumHeight(38);
        m_deleteButton->setVisible(false);
        m_deleteButton->setFocusPolicy(Qt::NoFocus);
        connect(m_deleteButton, &QPushButton::clicked, this, &EventDialog::onDeleteClicked);
        row->addWidget(m_deleteButton);
        row->addStretch();

        m_cancelButton = new QPushButton(I18n::t("event.cancel"));
        m_cancelButton->setMinimumSize(92, 38);
        m_cancelButton->setFocusPolicy(Qt::NoFocus);
        connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        row->addWidget(m_cancelButton);

        m_submitButton = new QPushButton(I18n::t("event.save"));
        m_submitButton->setProperty("class", "primary");
        m_submitButton->setMinimumSize(92, 38);
        m_submitButton->setDefault(true);
        m_submitButton->setFocusPolicy(Qt::NoFocus);
        connect(m_submitButton, &QPushButton::clicked, this, &EventDialog::onSubmit);
        row->addWidget(m_submitButton);

        root->addLayout(row);
    }
}

void EventDialog::applyTheme() {
    auto &t = Theme::instance();
    // V4.3 #2 — 显式给 QDialog 上背景色，不再依赖系统 palette；同时给 title
    // 上 textPrimary（不被 globalStylesheet 的 class="title" 选择器漏掉）。
    setStyleSheet(t.globalStylesheet() + QString(
        "QDialog#EventDialog { background-color: %1; }"
        "QLabel#EventDialogTitle { color: %2; background: transparent; }"
        "QWidget#EventDialogCategoryHost { background: transparent; }"
    ).arg(t.bgPage().name(), t.textPrimary().name()));

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
                "QPushButton{background:%1;color:%2;border:1px solid %3;border-radius:16px;"
                "padding:5px 16px;font-weight:600;outline:0;}"
            ).arg(brandLight, theme.brand().name(), theme.brand().name()));
        } else {
            btn->setStyleSheet(QString(
                "QPushButton{background:transparent;color:%1;border:1px solid %2;"
                "border-radius:16px;padding:5px 16px;outline:0;}"
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
                "QPushButton{background:%1;color:white;border:none;border-radius:16px;"
                "padding:5px 20px;font-weight:600;outline:0;}"
            ).arg(tints[i]));
        } else {
            btn->setStyleSheet(QString(
                "QPushButton{background:transparent;color:%1;border:1px solid %2;"
                "border-radius:16px;padding:5px 20px;outline:0;}"
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
        // V4.3 #3 — pal[cols[i]].text 现在对 12 种颜色都是有效色（Theme.cpp 已补齐）。
        QColor c = pal[cols[i]].text;
        if (!c.isValid()) c = QColor("#888888");   // 兜底
        if (active) {
            btn->setStyleSheet(QString(
                "QPushButton{background:%1;border:3px solid %2;border-radius:15px;outline:0;}"
            ).arg(c.name(), Theme::instance().textPrimary().name()));
        } else {
            btn->setStyleSheet(QString(
                "QPushButton{background:%1;border:1px solid %2;border-radius:15px;outline:0;}"
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
        QMessageBox::information(this, I18n::t("common.info"), I18n::t("event.title_required"));
        m_titleEdit->setFocus();
        return;
    }
    if (m_endEdit->dateTime() <= m_startEdit->dateTime()) {
        QMessageBox::information(this, I18n::t("common.info"), I18n::t("event.end_before_start"));
        return;
    }
    accept();
}

void EventDialog::onDeleteClicked() {
    auto ret = QMessageBox::question(this, I18n::t("event.delete_title"),
        I18n::t("event.delete_confirm_fmt").arg(m_titleEdit->text().trimmed()),
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
