//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "CategoryPieChart.h"
#include "Theme.h"
#include "../core/I18n.h"
#include "../core/Types.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace timemaster {

CategoryPieChart::CategoryPieChart(QWidget *parent) : QWidget(parent) {
    setMinimumSize(200, 200);
}

void CategoryPieChart::setStats(const QList<CategoryStat> &stats) {
    m_stats = stats;
    update();
}

void CategoryPieChart::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();
    auto pal = theme.palette();

    int side = qMin(width(), height()) - 24;
    QRect r((width() - side) / 2, (height() - side) / 2, side, side);

    if (m_stats.isEmpty() || m_stats.size() == 1) {
        qint64 total = 0;
        for (const auto &s : m_stats) total += s.totalMinutes;
        if (total <= 0) {
            p.setPen(theme.textPlaceholder());
            p.drawText(rect(), Qt::AlignCenter, timemaster::I18n::t("widget.no_data"));
            return;
        }
        if (m_stats.size() == 1) {
            auto color = pal[categoryDefaultColor(m_stats[0].category)].text;
            p.setBrush(color);
            p.setPen(Qt::NoPen);
            p.drawEllipse(r);
        }
    } else {
        qint64 total = 0;
        for (const auto &s : m_stats) total += s.totalMinutes;
        if (total <= 0) {
            p.setPen(theme.textPlaceholder());
            p.drawText(rect(), Qt::AlignCenter, timemaster::I18n::t("widget.no_data"));
            return;
        }
        int startAngle = 90 * 16;
        for (const auto &s : m_stats) {
            int spanAngle = -int(round(double(s.totalMinutes) / total * 360.0 * 16));
            QColor c = pal[categoryDefaultColor(s.category)].text;
            p.setBrush(c);
            p.setPen(QPen(theme.bgContainer(), 2));
            p.drawPie(r, startAngle, spanAngle);
            startAngle += spanAngle;
        }
    }

    int innerSide = side * 6 / 10;
    QRect inner((width() - innerSide) / 2, (height() - innerSide) / 2, innerSide, innerSide);
    p.setBrush(theme.bgContainer());
    p.setPen(Qt::NoPen);
    p.drawEllipse(inner);
}

} // namespace timemaster
