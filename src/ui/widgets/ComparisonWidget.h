//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

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
    void applyLanguage();

private:
    Database *m_db;

    QLabel *m_title = nullptr;
    QLabel *m_pastHeader = nullptr;
    QLabel *m_futHeader  = nullptr;
    QLabel *m_pastCap1   = nullptr;
    QLabel *m_pastCap2   = nullptr;
    QLabel *m_futCap1    = nullptr;
    QLabel *m_futCap2    = nullptr;
    QLabel *m_pastCountVal = nullptr;
    QLabel *m_pastHoursVal = nullptr;
    QLabel *m_futCountVal  = nullptr;
    QLabel *m_futHoursVal  = nullptr;
    QLabel *m_deltaLabel   = nullptr;

    QFrame *m_pastCard = nullptr;
    QFrame *m_futCard  = nullptr;
};

} // namespace timemaster
