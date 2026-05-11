#include "DailyTrendChart.h"
#include "../Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <algorithm>

namespace timemaster {

DailyTrendChart::DailyTrendChart(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(180);
    setAttribute(Qt::WA_StyledBackground, false);
    connect(&Theme::instance(), &Theme::changed, this, QOverload<>::of(&QWidget::update));
}

void DailyTrendChart::setData(const QList<DailySummary> &daily) {
    m_daily = daily;
    std::sort(m_daily.begin(), m_daily.end(), [](const DailySummary &a, const DailySummary &b) {
        return a.date < b.date;
    });
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

    qint64 maxV = 1;
    for (const auto &d : m_daily) if (d.totalMinutes > maxV) maxV = d.totalMinutes;

    const int padL = 36;
    const int padR = 16;
    const int padT = 14;
    const int padB = 30;
    const int chartW = std::max(1, width() - padL - padR);
    const int chartH = std::max(1, height() - padT - padB);

    // 网格
    p.setPen(QPen(theme.stroke(), 1, Qt::SolidLine));
    for (int i = 0; i <= 4; ++i) {
        int y = padT + chartH * i / 4;
        p.drawLine(padL, y, padL + chartW, y);

        // Y 轴刻度
        qint64 v = maxV - (maxV * i / 4);
        QString text;
        if (v >= 60) text = QString("%1h").arg(v / 60);
        else text = QString("%1m").arg(v);
        p.setPen(theme.textPlaceholder());
        QRect tr(0, y - 8, padL - 4, 16);
        p.drawText(tr, Qt::AlignVCenter | Qt::AlignRight, text);
        p.setPen(QPen(theme.stroke(), 1, Qt::SolidLine));
    }

    const int n = m_daily.size();
    auto xFor = [&](int i) {
        if (n <= 1) return padL + chartW / 2;
        return padL + int(double(i) / double(n - 1) * chartW);
    };
    auto yFor = [&](qint64 v) {
        return padT + chartH - int(double(v) / double(maxV) * chartH);
    };

    // 面积
    QPainterPath area;
    area.moveTo(xFor(0), padT + chartH);
    for (int i = 0; i < n; ++i) {
        area.lineTo(xFor(i), yFor(m_daily[i].totalMinutes));
    }
    area.lineTo(xFor(n - 1), padT + chartH);
    area.closeSubpath();

    QColor areaColor = theme.brand();
    QLinearGradient grad(0, padT, 0, padT + chartH);
    QColor c0 = areaColor; c0.setAlpha(96);
    QColor c1 = areaColor; c1.setAlpha(8);
    grad.setColorAt(0.0, c0);
    grad.setColorAt(1.0, c1);
    p.fillPath(area, grad);

    // 折线
    QPainterPath line;
    line.moveTo(xFor(0), yFor(m_daily[0].totalMinutes));
    for (int i = 1; i < n; ++i) {
        line.lineTo(xFor(i), yFor(m_daily[i].totalMinutes));
    }
    p.setPen(QPen(theme.brand(), 2.2));
    p.drawPath(line);

    // 点 + X 轴日期
    QFont xFont = font();
    xFont.setPointSize(8);
    p.setFont(xFont);

    int labelStep = std::max(1, n / 8); // 防止 X 轴 label 挤在一起
    for (int i = 0; i < n; ++i) {
        int x = xFor(i);
        int y = yFor(m_daily[i].totalMinutes);
        p.setBrush(theme.brand());
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPoint(x, y), 3, 3);
        p.setBrush(Qt::NoBrush);

        if (i % labelStep == 0 || i == n - 1) {
            p.setPen(theme.textPlaceholder());
            QString label = m_daily[i].date.toString("M/d");
            QRect lr(x - 30, padT + chartH + 6, 60, 16);
            p.drawText(lr, Qt::AlignCenter, label);
        }
    }
}

} // namespace timemaster
