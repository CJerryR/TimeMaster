#pragma once

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include "../../core/Types.h"

namespace timemaster {

class Database;

/**
 * 「过去一周 vs 未来一周」对比卡片
 *  - 左：过去 7 天（已完成）
 *  - 右：未来 7 天（待安排）
 *  - 中间一条小指示：节奏对比（更密 / 更松 / 持平）
 */
class ComparisonWidget : public QWidget {
    Q_OBJECT
public:
    explicit ComparisonWidget(Database *db, QWidget *parent = nullptr);

    void refresh();

private slots:
    void applyTheme();

private:
    Database *m_db;

    QLabel *m_title;
    QLabel *m_pastCountVal;
    QLabel *m_pastHoursVal;
    QLabel *m_futCountVal;
    QLabel *m_futHoursVal;
    QLabel *m_deltaLabel;

    QFrame *m_pastCard;
    QFrame *m_futCard;
};

} // namespace timemaster
