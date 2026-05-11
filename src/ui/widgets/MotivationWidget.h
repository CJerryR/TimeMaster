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
class MotivationWidget : public QWidget {
    Q_OBJECT
public:
    explicit MotivationWidget(Database *db, QWidget *parent = nullptr);

    void refresh(const QDateTime &start, const QDateTime &end);

private slots:
    void applyTheme();

private:
    QString pickDailyQuote() const;
    QString buildInsight(const QDateTime &start, const QDateTime &end) const;

    Database *m_db;
    QLabel *m_quoteLab;
    QLabel *m_attributeLab;
    QLabel *m_insightLab;
};

} // namespace timemaster
