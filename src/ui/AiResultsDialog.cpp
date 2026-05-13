//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "AiResultsDialog.h"
#include "EventDialog.h"
#include "Theme.h"
#include "IconRenderer.h"
#include "../core/I18n.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QFrame>
#include <QDialogButtonBox>

namespace timemaster {

// ----- 单条解析建议的行 widget -----
class AiSuggestionRow : public QFrame {
public:
    // 构造函数：勾选框 + 标题/元信息 + 编辑/删除按钮
    AiSuggestionRow(QWidget *parent = nullptr) : QFrame(parent) {
        setObjectName("AiSuggestionRow");
        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(12, 10, 12, 10);
        lay->setSpacing(10);

        check = new QCheckBox;
        check->setChecked(true);
        lay->addWidget(check);

        auto *col = new QVBoxLayout;
        col->setContentsMargins(0, 0, 0, 0);
        col->setSpacing(3);
        titleLab = new QLabel;
        titleLab->setObjectName("AiRowTitle");
        col->addWidget(titleLab);
        metaLab = new QLabel;
        metaLab->setObjectName("AiRowMeta");
        metaLab->setWordWrap(true);
        col->addWidget(metaLab);
        lay->addLayout(col, 1);

        editBtn = new QPushButton;
        editBtn->setObjectName("AiRowIconBtn");
        editBtn->setToolTip(I18n::t("ai.results.edit_tip"));
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setFixedSize(32, 32);
        lay->addWidget(editBtn);

        delBtn = new QPushButton;
        delBtn->setObjectName("AiRowIconBtnDanger");
        delBtn->setToolTip(I18n::t("ai.results.remove_tip"));
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setFixedSize(32, 32);
        lay->addWidget(delBtn);
    }

    // 用建议数据填充行内容：标题 / 时间 / 类别 / 优先级
    void setSuggestion(const ScheduleSuggestion &s) {
        titleLab->setText(s.title.isEmpty() ? I18n::t("ai.results.untitled") : s.title);
        QString time = s.allDay
            ? I18n::t("history.all_day")
            : (s.startDate.toString("MM-dd ddd HH:mm") + " — " + s.endDate.toString("HH:mm"));
        QString meta = time + "  ·  " + categoryLabel(s.category)
                     + "  ·  " + priorityLabel(s.priority);
        metaLab->setText(meta);
    }

    // 应用主题样式：设置行背景、文字颜色、按钮图标
    void applyStyle() {
        auto &t = Theme::instance();
        QColor fg = t.textSecondary();
        editBtn->setIcon(IconRenderer::icon(IconRenderer::Edit, fg, 18));
        delBtn->setIcon(IconRenderer::icon(IconRenderer::Delete, t.danger(), 18));
        setStyleSheet(QString(R"(
            QFrame#AiSuggestionRow {
                background: %1;
                border: 1px solid %2;
                border-radius: 10px;
            }
            QLabel#AiRowTitle {
                color: %3;
                font-size: 14px;
                font-weight: 600;
            }
            QLabel#AiRowMeta {
                color: %4;
                font-size: 12px;
            }
            QPushButton#AiRowIconBtn, QPushButton#AiRowIconBtnDanger {
                background: transparent;
                border: 1px solid %2;
                border-radius: 8px;
            }
            QPushButton#AiRowIconBtn:hover {
                background: %5;
            }
            QPushButton#AiRowIconBtnDanger:hover {
                background: rgba(184,69,62,0.10);
                border-color: %6;
            }
        )")
        .arg(t.componentBgRgba())
        .arg(t.strokeRgba())
        .arg(t.textPrimary().name())
        .arg(t.textSecondary().name())
        .arg(t.cardBgHoverRgba())
        .arg(t.danger().name()));
    }

    QCheckBox  *check;
    QLabel     *titleLab;
    QLabel     *metaLab;
    QPushButton *editBtn;
    QPushButton *delBtn;
};

// ============================================================
// AiResultsDialog
// ============================================================

// 构造函数：接收建议列表，初始化勾选状态并构建 UI
AiResultsDialog::AiResultsDialog(QList<ScheduleSuggestion> items, QWidget *parent)
    : QDialog(parent), m_items(std::move(items))
{
    setWindowTitle(I18n::t("ai.results.title"));
    setObjectName("AiResultsDialog");   // V4.3 #2 — for theme-aware QSS
    setModal(true);
    setMinimumSize(640, 600);
    resize(680, 640);

    for (int i = 0; i < m_items.size(); ++i) m_checked << true;

    buildUi();
    connect(&Theme::instance(), &Theme::changed, this, &AiResultsDialog::applyTheme);
    applyTheme();
}

// 构建界面：标题行 + 统计摘要 + 全选按钮 + 滚动列表 + 底部确认取消
void AiResultsDialog::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 20, 22, 18);
    root->setSpacing(12);

    auto *headerRow = new QHBoxLayout;
    headerRow->setContentsMargins(0, 0, 0, 0);
    headerRow->setSpacing(8);
    auto *headerIcon = new QLabel;
    headerIcon->setFixedSize(20, 20);
    headerIcon->setPixmap(IconRenderer::pixmap(IconRenderer::Sparkle, Theme::instance().brand(), 20));
    headerRow->addWidget(headerIcon);
    auto *header = new QLabel(I18n::t("ai.results.found_fmt").arg(m_items.size()));
    header->setProperty("class", "title");
    headerRow->addWidget(header);
    headerRow->addStretch();
    root->addLayout(headerRow);

    m_summaryLabel = new QLabel(I18n::t("ai.results.subtitle"));
    m_summaryLabel->setProperty("class", "caption");
    m_summaryLabel->setWordWrap(true);
    root->addWidget(m_summaryLabel);

    auto *toolRow = new QHBoxLayout;
    m_toggleAllBtn = new QPushButton(I18n::t("ai.results.deselect_all"));
    m_toggleAllBtn->setObjectName("ToggleAllBtn");
    m_toggleAllBtn->setCursor(Qt::PointingHandCursor);
    connect(m_toggleAllBtn, &QPushButton::clicked, this, &AiResultsDialog::onToggleAll);
    toolRow->addWidget(m_toggleAllBtn);
    toolRow->addStretch();
    root->addLayout(toolRow);

    // 列表（滚动）
    m_scroll = new QScrollArea;
    m_scroll->setObjectName("AiResultsScroll");
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *container = new QWidget;
    m_rowsLayout = new QVBoxLayout(container);
    m_rowsLayout->setContentsMargins(2, 2, 2, 2);
    m_rowsLayout->setSpacing(8);
    m_rowsLayout->addStretch();
    m_scroll->setWidget(container);
    root->addWidget(m_scroll, 1);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    bb->button(QDialogButtonBox::Save)->setText(I18n::t("ai.results.confirm"));
    bb->button(QDialogButtonBox::Save)->setProperty("class", "primary");
    bb->button(QDialogButtonBox::Cancel)->setText(I18n::t("ai.results.cancel"));
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    root->addWidget(bb);

    rebuildRows();
}

// 重建所有行：清空后根据 m_items 和 m_checked 重绘
void AiResultsDialog::rebuildRows() {
    // 清空已有
    for (auto *w : m_rowWidgets) {
        m_rowsLayout->removeWidget(w);
        w->deleteLater();
    }
    m_rowWidgets.clear();

    // 移除 stretch
    while (m_rowsLayout->count() > 0) {
        auto *item = m_rowsLayout->takeAt(0);
        if (item->widget()) {
            // 这里其实不会有 widget，因为上面已经清了
        }
        delete item;
    }

    for (int i = 0; i < m_items.size(); ++i) {
        auto *row = new AiSuggestionRow;
        row->setSuggestion(m_items[i]);
        row->check->setChecked(m_checked[i]);
        connect(row->check, &QCheckBox::toggled, this, [this, i](bool v) {
            if (i < m_checked.size()) m_checked[i] = v;
        });
        connect(row->editBtn, &QPushButton::clicked, this, [this, i] { onEditRow(i); });
        connect(row->delBtn, &QPushButton::clicked, this, [this, i] { onDeleteRow(i); });
        m_rowWidgets.append(row);
        m_rowsLayout->addWidget(row);
    }
    m_rowsLayout->addStretch();

    // 重新应用样式
    for (AiSuggestionRow *r : m_rowWidgets) {
        r->applyStyle();
    }

    if (m_summaryLabel) {
        if (m_items.isEmpty()) {
            m_summaryLabel->setText(I18n::t("ai.results.all_removed"));
        } else {
            m_summaryLabel->setText(I18n::t("ai.results.subtitle"));
        }
    }
}

// 编辑第 index 条建议：打开 EventDialog 修改并更新
void AiResultsDialog::onEditRow(int index) {
    if (index < 0 || index >= m_items.size()) return;
    const auto &s = m_items[index];

    // 把 ScheduleSuggestion 包装成 CalendarEvent 喂给 EventDialog
    CalendarEvent ev;
    ev.id = "_preview_";        // 占位，不入库
    ev.title = s.title;
    ev.description = s.description;
    ev.startDate = s.startDate;
    ev.endDate = s.endDate;
    ev.allDay = s.allDay;
    ev.color = s.color;
    ev.category = s.category;
    ev.priority = s.priority;
    ev.source = EventSource::AiParse;
    ev.createdAt = QDateTime::currentDateTime();
    ev.updatedAt = ev.createdAt;

    EventDialog dlg(this);
    dlg.setupForEdit(ev);
    // 隐藏删除按钮（删除在外层 onDeleteRow 处理）
    if (auto *btn = dlg.findChild<QPushButton*>()) {
        // 注意：findChild 拿第一个，不一定对；EventDialog 内部直接控制
    }

    if (dlg.exec() != QDialog::Accepted) return;
    CalendarEvent r = dlg.result();
    ScheduleSuggestion u = s;
    u.title = r.title;
    u.description = r.description;
    u.startDate = r.startDate;
    u.endDate = r.endDate;
    u.allDay = r.allDay;
    u.color = r.color;
    u.category = r.category;
    u.priority = r.priority;
    u.durationMinutes = static_cast<int>(u.startDate.secsTo(u.endDate) / 60);
    m_items[index] = u;

    // 只更新这一行的显示
    if (index < m_rowWidgets.size()) {
        m_rowWidgets[index]->setSuggestion(u);
    }
}

// 删除第 index 条建议：移除并重建行列表
void AiResultsDialog::onDeleteRow(int index) {
    if (index < 0 || index >= m_items.size()) return;
    m_items.removeAt(index);
    m_checked.removeAt(index);
    rebuildRows();
}

// 全选 / 全不选切换：更新所有勾选框和按钮文字
void AiResultsDialog::onToggleAll() {
    bool anyUnchecked = false;
    for (bool v : m_checked) if (!v) { anyUnchecked = true; break; }
    bool target = anyUnchecked; // 有未勾的 → 全选；全勾的 → 全取消
    for (int i = 0; i < m_checked.size(); ++i) {
        m_checked[i] = target;
        if (i < m_rowWidgets.size()) {
            m_rowWidgets[i]->check->setChecked(target);
        }
    }
    m_toggleAllBtn->setText(target ? I18n::t("ai.results.deselect_all")
                                    : I18n::t("ai.results.select_all"));
}

// 获取所有勾选的建议列表
QList<ScheduleSuggestion> AiResultsDialog::selectedSuggestions() const {
    QList<ScheduleSuggestion> out;
    for (int i = 0; i < m_items.size(); ++i) {
        if (i < m_checked.size() && m_checked[i]) out << m_items[i];
    }
    return out;
}

// 应用主题：设置弹窗 QSS 并刷新所有行样式
void AiResultsDialog::applyTheme() {
    auto &t = Theme::instance();
    // V4.3 #2 — 同 EventDialog：显式设置 QDialog 背景，否则黑暗模式下 Qt 默认
    // 调系统 palette.window（浅色），整个弹窗"浅底深字"看起来很错位。
    setStyleSheet(t.globalStylesheet() + QString(R"(
        QDialog#AiResultsDialog { background-color: %5; }
        QScrollArea#AiResultsScroll {
            background: transparent;
            border: none;
        }
        QPushButton#ToggleAllBtn {
            background: transparent;
            border: 1px solid %1;
            border-radius: 8px;
            padding: 5px 12px;
            color: %2;
            font-size: 13px;
            outline: 0;
        }
        QPushButton#ToggleAllBtn:hover {
            background: %3;
            color: %4;
        }
    )")
    .arg(t.strokeRgba())
    .arg(t.textSecondary().name())
    .arg(t.cardBgHoverRgba())
    .arg(t.textPrimary().name())
    .arg(t.bgPage().name()));

    for (AiSuggestionRow *r : m_rowWidgets) {
        r->applyStyle();
    }
}

} // namespace timemaster
