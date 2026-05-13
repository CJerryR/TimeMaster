//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include <QDateTime>
#include "../../core/Types.h"

class QLabel;

namespace timemaster {

class Database;

// 数据洞察文案：基于统计数据生成连续天数/主要类别/日均投入等激励性文案
class InsightsWidget : public QWidget {
    Q_OBJECT
public:
    // 构造函数：初始化标题和文案标签
    explicit InsightsWidget(Database *db, QWidget *parent = nullptr);
    // 根据时间范围刷新洞察文案
    void refresh(const QDateTime &start, const QDateTime &end);

private:
    Database *m_db;
    QLabel *m_title = nullptr;
    QLabel *m_text  = nullptr;
    QDateTime m_lastStart;
    QDateTime m_lastEnd;
};

} // namespace timemaster
