//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QLabel;
class QPushButton;
class QTextBrowser;

namespace timemaster {

class Database;

/**
 * AI 导入历史面板
 *  - 左侧：批次列表（按时间倒序，显示每批的原始输入和事件数）
 *  - 右侧：当前选中批次的事件清单
 *  - 操作：
 *    · 撤销整批 → 删除该批次所有事件 + 批次记录
 *    · 单条删除 → 仅删除该事件（批次会更新存活计数）
 *    · 仅清理历史 → 保留事件，仅清除批次记录（适合"确认无误后归档"）
 */
class AiHistoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit AiHistoryDialog(Database *db, QWidget *parent = nullptr);

private slots:
    void applyTheme();
    void applyLanguage();
    void reloadBatches();
    void onBatchSelected();
    void onUndoBatch();
    void onArchiveBatch();
    void onDeleteEvent();

private:
    void buildUi();
    void renderRightPane();
    QString currentBatchId() const;

    Database *m_db;

    QListWidget *m_batchList;
    QListWidget *m_eventList;
    QLabel *m_emptyHint;
    QLabel *m_rightHeader;
    QTextBrowser *m_sourceTextView;
    QPushButton *m_undoBtn;
    QPushButton *m_archiveBtn;
    QPushButton *m_deleteEventBtn;
    QPushButton *m_closeBtn = nullptr;

    QLabel *m_titleLabel    = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLabel *m_leftHeader    = nullptr;
    QLabel *m_srcCaption    = nullptr;
    QLabel *m_evCaption     = nullptr;

    QList<AiBatchInfo> m_batches;
    QList<CalendarEvent> m_currentBatchEvents;
};

} // namespace timemaster
