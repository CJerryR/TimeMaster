#include "AiHistoryDialog.h"
#include "Theme.h"
#include "../core/Database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QFrame>
#include <QMessageBox>

namespace timemaster {

AiHistoryDialog::AiHistoryDialog(Database *db, QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setWindowTitle("AI 导入历史");
    setModal(true);
    resize(880, 580);
    setMinimumSize(740, 480);

    buildUi();
    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, &AiHistoryDialog::applyTheme);
    connect(m_db, &Database::aiBatchesChanged, this, &AiHistoryDialog::reloadBatches);

    reloadBatches();
}

void AiHistoryDialog::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 20, 22, 18);
    root->setSpacing(14);

    // ---- 标题区 ----
    auto *titleRow = new QHBoxLayout;
    auto *title = new QLabel("🕐  AI 导入历史");
    title->setObjectName("HistoryTitle");
    titleRow->addWidget(title);

    auto *subtitle = new QLabel("撤销 AI 误识别的日程");
    subtitle->setObjectName("HistorySubtitle");
    titleRow->addWidget(subtitle);
    titleRow->addStretch();

    root->addLayout(titleRow);

    // ---- 内容区：左右分栏 ----
    auto *contentRow = new QHBoxLayout;
    contentRow->setSpacing(14);

    // 左侧批次列表
    auto *leftPane = new QFrame;
    leftPane->setObjectName("HistoryPane");
    auto *leftLayout = new QVBoxLayout(leftPane);
    leftLayout->setContentsMargins(2, 2, 2, 2);
    leftLayout->setSpacing(0);

    auto *leftHeader = new QLabel("导入批次");
    leftHeader->setObjectName("PaneHeader");
    leftHeader->setContentsMargins(14, 12, 14, 8);
    leftLayout->addWidget(leftHeader);

    m_batchList = new QListWidget;
    m_batchList->setObjectName("BatchList");
    m_batchList->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    leftLayout->addWidget(m_batchList);

    m_emptyHint = new QLabel("还没有 AI 导入记录\n\n通过日历页顶部的「AI 解析」\n导入日程后会显示在这里");
    m_emptyHint->setObjectName("EmptyHint");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_emptyHint->setWordWrap(true);
    leftLayout->addWidget(m_emptyHint);
    m_emptyHint->hide();

    contentRow->addWidget(leftPane, 4);

    // 右侧详情
    auto *rightPane = new QFrame;
    rightPane->setObjectName("HistoryPane");
    auto *rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(14, 12, 14, 12);
    rightLayout->setSpacing(10);

    m_rightHeader = new QLabel("选择左侧的批次查看详情");
    m_rightHeader->setObjectName("PaneHeader");
    rightLayout->addWidget(m_rightHeader);

    auto *srcLabel = new QLabel("原始输入");
    srcLabel->setProperty("class", "caption");
    rightLayout->addWidget(srcLabel);

    m_sourceTextView = new QTextBrowser;
    m_sourceTextView->setObjectName("SourceTextView");
    m_sourceTextView->setMaximumHeight(80);
    m_sourceTextView->setReadOnly(true);
    rightLayout->addWidget(m_sourceTextView);

    auto *evLabel = new QLabel("批次内事件");
    evLabel->setProperty("class", "caption");
    rightLayout->addWidget(evLabel);

    m_eventList = new QListWidget;
    m_eventList->setObjectName("EventList");
    m_eventList->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    rightLayout->addWidget(m_eventList, 1);

    // 右侧底部按钮区
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);

    m_deleteEventBtn = new QPushButton("删除选中事件");
    m_deleteEventBtn->setObjectName("DangerGhostBtn");
    m_deleteEventBtn->setCursor(Qt::PointingHandCursor);
    m_deleteEventBtn->setEnabled(false);
    btnRow->addWidget(m_deleteEventBtn);

    btnRow->addStretch();

    m_archiveBtn = new QPushButton("仅清理历史记录");
    m_archiveBtn->setObjectName("SecondaryBtn");
    m_archiveBtn->setCursor(Qt::PointingHandCursor);
    m_archiveBtn->setToolTip("保留日历事件，只清掉这条导入记录");
    m_archiveBtn->setEnabled(false);
    btnRow->addWidget(m_archiveBtn);

    m_undoBtn = new QPushButton("↶ 撤销整批");
    m_undoBtn->setObjectName("DangerBtn");
    m_undoBtn->setCursor(Qt::PointingHandCursor);
    m_undoBtn->setToolTip("删除该批次的所有事件");
    m_undoBtn->setEnabled(false);
    btnRow->addWidget(m_undoBtn);

    rightLayout->addLayout(btnRow);

    contentRow->addWidget(rightPane, 6);

    root->addLayout(contentRow, 1);

    // ---- 底部关闭 ----
    auto *footer = new QHBoxLayout;
    footer->addStretch();
    auto *closeBtn = new QPushButton("关闭");
    closeBtn->setObjectName("CloseBtn");
    closeBtn->setMinimumWidth(90);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    footer->addWidget(closeBtn);
    root->addLayout(footer);

    // 信号
    connect(m_batchList, &QListWidget::itemSelectionChanged,
            this, &AiHistoryDialog::onBatchSelected);
    connect(m_eventList, &QListWidget::itemSelectionChanged,
            this, [this]{
                m_deleteEventBtn->setEnabled(m_eventList->currentItem() != nullptr);
            });
    connect(m_undoBtn, &QPushButton::clicked, this, &AiHistoryDialog::onUndoBatch);
    connect(m_archiveBtn, &QPushButton::clicked, this, &AiHistoryDialog::onArchiveBatch);
    connect(m_deleteEventBtn, &QPushButton::clicked, this, &AiHistoryDialog::onDeleteEvent);
}

void AiHistoryDialog::applyTheme() {
    auto &t = Theme::instance();
    QString textPrim = t.textPrimary().name();
    QString textSec = t.textSecondary().name();
    QString placeholder = t.textPlaceholder().name();
    QString brand = t.brand().name();
    QString danger = t.danger().name();
    QString strokeR = t.strokeRgba();
    QString cardBg = t.cardBgRgba();
    QString componentBg = t.componentBgRgba();
    QString hoverBg = t.cardBgHoverRgba();
    QString container = t.bgContainer().name();

    setStyleSheet(t.globalStylesheet() + QString(R"(
        QDialog {
            background-color: %10;
        }
        QLabel#HistoryTitle {
            font-size: 18px;
            font-weight: 700;
            color: %1;
        }
        QLabel#HistorySubtitle {
            color: %3;
            font-size: 13px;
            margin-left: 8px;
            margin-top: 4px;
        }
        QFrame#HistoryPane {
            background-color: %5;
            border: 1px solid %4;
            border-radius: 12px;
        }
        QLabel#PaneHeader {
            color: %1;
            font-size: 13px;
            font-weight: 600;
        }
        QLabel#EmptyHint {
            color: %3;
            font-size: 13px;
            line-height: 1.6;
            padding: 40px 16px;
        }
        QListWidget#BatchList, QListWidget#EventList {
            background-color: transparent;
            border: none;
            outline: 0;
        }
        QListWidget#BatchList::item {
            border-radius: 9px;
            margin: 2px 6px;
            padding: 10px 12px;
            color: %1;
        }
        QListWidget#BatchList::item:hover { background-color: %6; }
        QListWidget#BatchList::item:selected {
            background-color: rgba(239,68,68,0.12);
            color: %7;
        }
        QListWidget#EventList::item {
            border-radius: 8px;
            margin: 2px 4px;
            padding: 8px 10px;
            color: %1;
        }
        QListWidget#EventList::item:hover { background-color: %6; }
        QListWidget#EventList::item:selected {
            background-color: rgba(239,68,68,0.10);
        }

        QTextBrowser#SourceTextView {
            background-color: %2;
            border: 1px solid %4;
            border-radius: 9px;
            padding: 8px 10px;
            color: %1;
            font-size: 13px;
        }

        QPushButton#DangerBtn {
            background-color: %8;
            color: white;
            border: none;
            border-radius: 9px;
            padding: 8px 18px;
            font-weight: 600;
        }
        QPushButton#DangerBtn:hover { background-color: #b91c1c; }
        QPushButton#DangerBtn:disabled {
            background-color: %2;
            color: %3;
        }
        QPushButton#DangerGhostBtn {
            background-color: transparent;
            color: %8;
            border: 1px solid %4;
            border-radius: 9px;
            padding: 8px 14px;
        }
        QPushButton#DangerGhostBtn:hover {
            background-color: rgba(220,38,38,0.08);
            border-color: %8;
        }
        QPushButton#DangerGhostBtn:disabled {
            color: %3;
            border-color: %4;
        }
        QPushButton#SecondaryBtn {
            background-color: transparent;
            color: %3;
            border: 1px solid %4;
            border-radius: 9px;
            padding: 8px 14px;
        }
        QPushButton#SecondaryBtn:hover {
            background-color: %6;
            color: %1;
        }
        QPushButton#SecondaryBtn:disabled {
            color: %3;
            border-color: %4;
        }
        QPushButton#CloseBtn {
            background-color: %2;
            color: %1;
            border: 1px solid %4;
            border-radius: 9px;
            padding: 8px 18px;
        }
        QPushButton#CloseBtn:hover { background-color: %6; }
    )")
    /*1*/.arg(textPrim)
    /*2*/.arg(componentBg)
    /*3*/.arg(textSec)
    /*4*/.arg(strokeR)
    /*5*/.arg(cardBg)
    /*6*/.arg(hoverBg)
    /*7*/.arg(brand)
    /*8*/.arg(danger)
    /*9*/.arg(placeholder)
    /*10*/.arg(container));
}

void AiHistoryDialog::reloadBatches() {
    m_batches = m_db->getAiBatches();
    QString prevId = currentBatchId();
    m_batchList->clear();

    if (m_batches.isEmpty()) {
        m_emptyHint->show();
        m_batchList->hide();
        m_rightHeader->setText("选择左侧的批次查看详情");
        m_sourceTextView->clear();
        m_eventList->clear();
        m_currentBatchEvents.clear();
        m_undoBtn->setEnabled(false);
        m_archiveBtn->setEnabled(false);
        m_deleteEventBtn->setEnabled(false);
        return;
    }

    m_emptyHint->hide();
    m_batchList->show();

    int restoreIdx = -1;
    for (int i = 0; i < m_batches.size(); ++i) {
        const auto &b = m_batches[i];
        auto *item = new QListWidgetItem();
        QString relTime = b.createdAt.toString("MM-dd HH:mm");
        QString preview = b.sourceText;
        preview.replace('\n', ' ');
        if (preview.length() > 40) preview = preview.left(37) + "…";
        QString badge = QString("· %1 条").arg(b.aliveCount);
        QString text = QString("%1\n%2  %3").arg(relTime, preview, badge);
        item->setText(text);
        item->setData(Qt::UserRole, b.id);
        item->setSizeHint(QSize(0, 56));
        m_batchList->addItem(item);
        if (b.id == prevId) restoreIdx = i;
    }

    if (restoreIdx >= 0) {
        m_batchList->setCurrentRow(restoreIdx);
    } else if (!m_batches.isEmpty()) {
        m_batchList->setCurrentRow(0);
    }
}

QString AiHistoryDialog::currentBatchId() const {
    auto *it = m_batchList->currentItem();
    return it ? it->data(Qt::UserRole).toString() : QString();
}

void AiHistoryDialog::onBatchSelected() {
    QString id = currentBatchId();
    if (id.isEmpty()) return;

    AiBatchInfo current;
    for (const auto &b : m_batches) if (b.id == id) { current = b; break; }

    m_rightHeader->setText(QString("批次详情  ·  %1").arg(current.createdAt.toString("yyyy-MM-dd HH:mm")));
    m_sourceTextView->setPlainText(current.sourceText);

    m_currentBatchEvents = m_db->getBatchEvents(id);
    m_eventList->clear();
    auto pal = Theme::instance().palette();
    for (const auto &e : m_currentBatchEvents) {
        auto *item = new QListWidgetItem();
        QString time = e.allDay
            ? "全天"
            : (e.startDate.toString("MM-dd HH:mm")
               + " — " + e.endDate.toString("HH:mm"));
        item->setText(QString("  %1\n  %2 · %3")
                          .arg(e.title, time, categoryLabel(e.category)));
        QColor c = pal[e.color].text;
        item->setForeground(c);
        item->setData(Qt::UserRole, e.id);
        item->setSizeHint(QSize(0, 50));
        m_eventList->addItem(item);
    }

    bool hasEvents = !m_currentBatchEvents.isEmpty();
    m_undoBtn->setEnabled(hasEvents);
    m_archiveBtn->setEnabled(true);
    m_deleteEventBtn->setEnabled(false);
}

void AiHistoryDialog::onUndoBatch() {
    QString id = currentBatchId();
    if (id.isEmpty()) return;

    int n = m_currentBatchEvents.size();
    auto ret = QMessageBox::question(this, "确认撤销",
        QString("将从日历中删除这一批 %1 个事件，且不可恢复。是否继续？").arg(n),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    m_db->deleteBatch(id);
    // reloadBatches() 会被 aiBatchesChanged 触发
}

void AiHistoryDialog::onArchiveBatch() {
    QString id = currentBatchId();
    if (id.isEmpty()) return;

    auto ret = QMessageBox::question(this, "清理历史记录",
        "保留这些日历事件，仅清掉这条 AI 导入记录。是否继续？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    m_db->clearBatchRecord(id);
}

void AiHistoryDialog::onDeleteEvent() {
    auto *it = m_eventList->currentItem();
    if (!it) return;
    QString eid = it->data(Qt::UserRole).toString();
    if (eid.isEmpty()) return;

    auto ret = QMessageBox::question(this, "确认删除",
        "从日历中删除这条事件？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    m_db->deleteEvent(eid);
    // 刷新当前批次详情
    onBatchSelected();
}

} // namespace timemaster
