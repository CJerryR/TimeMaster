#pragma once

#include <QWidget>

class QLabel;

namespace timeplan {

class SourceBar : public QWidget {
    Q_OBJECT
public:
    explicit SourceBar(QWidget *parent = nullptr);
    void setSources(int manual, int ai, int imported);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int m_manual = 0, m_ai = 0, m_import = 0;
    void drawSegment(QPainter &p, int x, int y, int w, int h, QColor c, bool left);
    void drawLegend(QPainter &p, int x, int y, QColor c, const QString &txt);
};

class SourceDistributionWidget : public QWidget {
    Q_OBJECT
public:
    explicit SourceDistributionWidget(QWidget *parent = nullptr);
    void setSources(int manual, int ai, int imported);

private:
    QLabel *m_title;
    SourceBar *m_bar;
};

} // namespace timeplan
