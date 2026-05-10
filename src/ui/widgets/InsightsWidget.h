#pragma once

#include <QWidget>
#include <QDateTime>
#include "../../core/Types.h"

class QLabel;

namespace timeplan {

class Database;

class InsightsWidget : public QWidget {
    Q_OBJECT
public:
    explicit InsightsWidget(Database *db, QWidget *parent = nullptr);
    void refresh(const QDateTime &start, const QDateTime &end);

private:
    Database *m_db;
    QLabel *m_title;
    QLabel *m_text;
};

} // namespace timeplan
