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

// 过去 vs 未来对比面板：双栏显示过去7天和未来7天的事件数/总时长/忙闲变化
/**
 * 「过去一周 vs 未来一周」对比卡片
 *  - 左：过去 7 天（已完成）
 *  - 右：未来 7 天（待安排）
 *  - 中间一条小指示：节奏对比（更密 / 更松 / 持平）
 */
class ComparisonWidget : public QWidget {
    Q_OBJECT
public:
    // 构造函数：创建双栏对比布局（过去/未来）+ 中间对比指示标签
    explicit ComparisonWidget(Database *db, QWidget *parent = nullptr);
    // 查询过去7天和未来7天数据，刷新事件数/总时长/忙闲对比
    void refresh();

private slots:
    // 主题变更时重新应用对比面板 QSS 样式
    void applyTheme();
    // 语言切换时更新所有文本标签并刷新数据
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
