#include "DailyTrendChart.h"
#include "../Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QDate>
#include <QtMath>

namespace timemaster {

DailyTrendChart::DailyTrendChart(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(160);
}

void DailyTrendChart::setData(const QList<DailySummary> &daily) {
    m_daily = daily;
    update();
}

void DailyTrendChart::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();

    if (m_daily.isEmpty()) {
        p.setPen(theme.textPlaceholder());
        p.drawText(rect(), Qt::AlignCenter, "暂无数据");
        return;
    }

    QMargins m(40, 20, 20, 28);
    QRect chartR = rect().marginsRemoved(m);

    qint64 maxMin = 0;
    for (const auto &d : m_daily)
        if (d.totalMinutes > maxMin) maxMin = d.totalMinutes;
    if (maxMin <= 0) maxMin = 1;

    int n = m_daily.size();
    double barPad = 0.3;
    double slotW = double(chartR.width()) / n;
    double barW = slotW * (1.0 - barPad);

    QColor barColor = theme.brand();
    QColor barColorLighter = barColor.lighter(140);

    for (int i = 0; i < n; ++i) {
        double x = chartR.left() + slotW * i + slotW * barPad / 2.0;
        double h = double(m_daily[i].totalMinutes) / maxMin * chartR.height();
        if (h < 2) h = 2;
        double y = chartR.bottom() - h;

        QRectF barR(x, y, barW, h);
        QPainterPath pp;
        pp.addRoundedRect(barR, 3, 3);

        bool isToday = (m_daily[i].date == QDate::currentDate());
        p.fillPath(pp, isToday ? barColor : barColorLighter);
    }

    p.setPen(theme.textSecondary());
    QFont labelFont = font();
    labelFont.setPointSize(9);
    p.setFont(labelFont);

    int skip = qMax(1, n / 7);
    for (int i = 0; i < n; i += skip) {
        double x = chartR.left() + slotW * i + slotW / 2.0;
        QString label = m_daily[i].date.toString("M/d");
        QRectF lr(x - 18, chartR.bottom() + 4, 36, 16);
        p.drawText(lr, Qt::AlignCenter, label);
    }

    qint64 steps[] = {0, maxMin / 2, maxMin};
    for (auto s : steps) {
        if (s == 0 && maxMin <= 60) continue;
        double y = chartR.bottom() - double(s) / maxMin * chartR.height();
        QRectF lr(0, y - 8, m.left() - 4, 16);
        qint64 hh = s / 60;
        QString txt = hh > 0 ? QString("%1h").arg(hh) : QString("%1m").arg(s);
        p.drawText(lr, Qt::AlignRight | Qt::AlignVCenter, txt);
    }
}

} // namespace timemaster
