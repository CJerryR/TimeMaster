#include "SourceDistributionWidget.h"
#include "../Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>
#include <QtMath>

namespace timeplan {

SourceDistributionWidget::SourceDistributionWidget(QWidget *parent) : QWidget(parent) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    m_title = new QLabel("来源分布");
    m_title->setProperty("class", "subtitle");
    lay->addWidget(m_title);
    m_bar = new SourceBar();
    lay->addWidget(m_bar, 1);
}

void SourceDistributionWidget::setSources(int manual, int ai, int imported) {
    m_bar->setSources(manual, ai, imported);
}

// ---- SourceBar ----

SourceBar::SourceBar(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(40);
}

void SourceBar::setSources(int manual, int ai, int imported) {
    m_manual = manual;
    m_ai = ai;
    m_import = imported;
    update();
}

void SourceBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();
    int total = m_manual + m_ai + m_import;
    if (total <= 0) {
        p.setPen(theme.textPlaceholder());
        p.drawText(rect(), Qt::AlignCenter, "暂无数据");
        return;
    }

    int h = 28;
    int y = (height() - h) / 2;
    int w = width() - 20;
    int x = 10;
    double rManual = double(m_manual) / total;
    double rAi = double(m_ai) / total;
    double rImport = double(m_import) / total;

    int wManual = int(rManual * w);
    int wAi = int(rAi * w);
    int wImport = w - wManual - wAi;

    QColor cManual = theme.brand();
    QColor cAi = theme.brand().lighter(130);
    QColor cImport = theme.brand().lighter(160);

    drawSegment(p, x, y, wManual, h, cManual, true);
    x += wManual;
    drawSegment(p, x, y, wAi, h, cAi, false);
    x += wAi;
    if (wImport > 0)
        drawSegment(p, x, y, wImport, h, cImport, false);

    QFont f = font();
    f.setPointSize(10);
    p.setFont(f);

    int legendY = y + h + 12;
    int legendX = 10;
    drawLegend(p, legendX, legendY, cManual, QString("手动 (%1%)").arg(int(rManual * 100)));
    legendX += 80;
    if (m_ai > 0) {
        drawLegend(p, legendX, legendY, cAi, QString("AI 解析 (%1%)").arg(int(rAi * 100)));
        legendX += 90;
    }
    if (m_import > 0)
        drawLegend(p, legendX, legendY, cImport, QString("导入 (%1%)").arg(int(rImport * 100)));
}

void SourceBar::drawSegment(QPainter &p, int x, int y, int w, int h, QColor c, bool left) {
    if (w <= 0) return;
    QPainterPath path;
    if (left)
        path.addRoundedRect(QRectF(x, y, w, h), 6, 6);
    else
        path.addRect(QRectF(x, y, w, h));
    path.addRoundedRect(QRectF(x, y, w, h), 6, 6);
    p.fillPath(path, c);
}

void SourceBar::drawLegend(QPainter &p, int x, int y, QColor c, const QString &txt) {
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawRect(QRect(x, y + 2, 10, 10));
    p.setPen(Theme::instance().textSecondary());
    p.drawText(QRect(x + 14, y, 100, 16), Qt::AlignLeft | Qt::AlignVCenter, txt);
}

} // namespace timeplan
