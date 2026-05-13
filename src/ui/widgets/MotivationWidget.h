//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include <QLabel>
#include "../../core/Types.h"

namespace timemaster {

class Database;

/**
 * 「Daily slogan + 一句数据洞察」组件
 *  - 第一行：每日一句固定语录（语录池随日期切换）
 *  - 第二行：基于用户数据动态生成（连续天数 / 工作占比 / 最忙日等）
 *  - 风格参考 LeetCode 顶部 Daily Quote，端正、克制、有力
 */
// 励志 Slogan 组件：每日一句名言（中英双语池15条）+ 数据洞察短句
class MotivationWidget : public QWidget {
    Q_OBJECT
public:
    // 构造函数：创建名言/出处/洞察三层标签并连接信号
    explicit MotivationWidget(Database *db, QWidget *parent = nullptr);
    // 刷新每日名言和数据洞察文案
    void refresh(const QDateTime &start, const QDateTime &end);

private slots:
    // 主题变更时刷新样式
    void applyTheme();

private:
    // 按 day-of-year 稳定选取每日名言
    QString pickDailyQuote() const;
    // 根据统计数据生成数据洞察短句
    QString buildInsight(const QDateTime &start, const QDateTime &end) const;

    Database *m_db;
    QLabel *m_quoteLab;
    QLabel *m_attributeLab;
    QLabel *m_insightLab;
};

} // namespace timemaster
