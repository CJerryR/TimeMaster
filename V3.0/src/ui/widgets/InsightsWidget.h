#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QList>

namespace timemaster {

class Database;

/**
 * 智能洞察：从数据中归纳出几条人类可读的发现。
 * 例如："你这段时间工作时长是学习的 2.3 倍"、"周三是你的高产日" 等。
 * 不调用 AI，纯本地启发式。
 */
class InsightsWidget : public QWidget {
    Q_OBJECT
public:
    explicit InsightsWidget(Database *db, QWidget *parent = nullptr);

    void refresh(const QDateTime &start, const QDateTime &end);

private:
    struct InsightItem {
        QString icon;
        QString text;
    };

    QList<InsightItem> compute(const QDateTime &start, const QDateTime &end);
    void rebuild(const QList<InsightItem> &items);
    void applyTheme();
    void clearItems();

    Database *m_db = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_subtitle = nullptr;
    QVBoxLayout *m_itemsLayout = nullptr;
    QList<QWidget *> m_itemWidgets;
};

} // namespace timemaster
