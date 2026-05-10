#include "DailyTrendChart.h"
#include "../Theme.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <algorithm>

namespace cadence {

DailyTrendChart::DailyTrendChart(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(180);
}

QSize DailyTrendChart::sizeHint() const {
    return QSize(700, 200);
}

void DailyTrendChart::setData(const QVector<TrendPoint>& data) {
    m_data = data;
    update();
}

void DailyTrendChart::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& th = Theme::instance();

    if (m_data.isEmpty()) {
        p.setPen(th.textPlaceholder());
        QFont f = font(); f.setPixelSize(13); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "暂无数据");
        return;
    }

    int maxCount = 0;
    int total = 0;
    for (const auto& d : m_data) {
        maxCount = std::max(maxCount, d.count);
        total += d.count;
    }
    if (maxCount == 0) maxCount = 1;
    const double avg = m_data.isEmpty() ? 0 : (double)total / m_data.size();

    const int n = m_data.size();
    const int padTop = 18;
    const int padBottom = 28;
    const int padLeft = 28;
    const int padRight = 8;
    const int chartH = height() - padTop - padBottom;
    const int chartW = width() - padLeft - padRight;
    if (chartW <= 0 || chartH <= 0) return;

    const double slotW = (double)chartW / n;
    const int barW = std::max(3, (int)(slotW * 0.65));

    // y 轴最大值刻度 (左上角)
    QFont scaleFont = font(); scaleFont.setPixelSize(10);
    p.setFont(scaleFont);
    p.setPen(th.textPlaceholder());
    p.drawText(QRect(0, padTop - 12, padLeft - 4, 12),
               Qt::AlignRight | Qt::AlignVCenter, QString::number(maxCount));
    p.drawText(QRect(0, padTop + chartH - 4, padLeft - 4, 12),
               Qt::AlignRight | Qt::AlignVCenter, "0");

    // 平均线
    if (avg > 0) {
        int avgY = padTop + chartH - (int)(chartH * avg / maxCount);
        QPen avgPen(th.textPlaceholder());
        avgPen.setStyle(Qt::DashLine);
        avgPen.setWidthF(1.0);
        p.setPen(avgPen);
        p.drawLine(padLeft, avgY, padLeft + chartW, avgY);
        p.setPen(th.textPlaceholder());
        QString avgLabel = QString("均 %1").arg(avg, 0, 'f', 1);
        p.drawText(QRect(padLeft + chartW - 60, avgY - 14, 60, 12),
                   Qt::AlignRight | Qt::AlignBottom, avgLabel);
    }

    // 柱子
    for (int i = 0; i < n; ++i) {
        const auto& d = m_data[i];
        double slotCx = padLeft + slotW * (i + 0.5);
        int barH = (d.count > 0) ? std::max(2, (int)(chartH * d.count / maxCount)) : 0;

        if (barH > 0) {
            QRect bar((int)(slotCx - barW / 2.0), padTop + chartH - barH, barW, barH);
            QColor c = (d.count == maxCount) ? th.brand() : th.brand().lighter(140);
            p.setPen(Qt::NoPen);
            p.setBrush(c);
            p.drawRoundedRect(bar, 2, 2);

            // 高条上方写数字
            if (d.count == maxCount && d.count > 0) {
                p.setFont(scaleFont);
                p.setPen(th.textPrimary());
                p.drawText(QRect((int)(slotCx - 20), bar.y() - 14, 40, 12),
                           Qt::AlignCenter, QString::number(d.count));
            }
        }

        // 日期标签：每周一(周一 == 1) 或第一/最后一个
        bool showDate = (d.date.dayOfWeek() == 1) || (i == 0) || (i == n - 1);
        if (showDate) {
            p.setFont(scaleFont);
            p.setPen(th.textPlaceholder());
            QString dateLabel = d.date.toString("M/d");
            p.drawText(QRect((int)(slotCx - 24), padTop + chartH + 4, 48, 14),
                       Qt::AlignCenter, dateLabel);
        }
    }
}

} // namespace cadence
