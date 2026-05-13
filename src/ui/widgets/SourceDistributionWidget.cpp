//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "SourceDistributionWidget.h"
#include "../Theme.h"
#include "../../core/I18n.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>
#include <QtMath>

namespace timemaster {

// 构造函数：创建标题标签和来源条形图组件
SourceDistributionWidget::SourceDistributionWidget(QWidget *parent) : QWidget(parent) {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    m_title = new QLabel(I18n::t("widget.source"));
    m_title->setProperty("class", "subtitle");
    lay->addWidget(m_title);
    m_bar = new SourceBar();
    lay->addWidget(m_bar, 1);
    connect(&I18n::instance(), &I18n::languageChanged, this, [this]{
        if (m_title) m_title->setText(I18n::t("widget.source"));
        if (m_bar) m_bar->update();
    });
}

// 转发三类数据给内部 SourceBar
void SourceDistributionWidget::setSources(int manual, int ai, int imported) {
    m_bar->setSources(manual, ai, imported);
}

// ---- SourceBar ----

// 构造函数：设置最小高度 72px
SourceBar::SourceBar(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(72);   // V4.2: was 40, room for bar + legend below
}

// 设置三类来源计数并触发重绘
void SourceBar::setSources(int manual, int ai, int imported) {
    m_manual = manual;
    m_ai = ai;
    m_import = imported;
    update();
}

// 自绘：按比例绘制三段彩色条形及下方图例
void SourceBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();
    int total = m_manual + m_ai + m_import;
    if (total <= 0) {
        p.setPen(theme.textPlaceholder());
        p.drawText(rect(), Qt::AlignCenter, timemaster::I18n::t("widget.no_data"));
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
    f.setPointSize(11);   // V4.2: was 10, slightly bigger for legend area
    p.setFont(f);

    int legendY = y + h + 14;
    int legendX = 10;
    drawLegend(p, legendX, legendY, cManual,
        timemaster::I18n::t("widget.source.manual_fmt").arg(int(rManual * 100)));
    legendX += 90;
    if (m_ai > 0) {
        drawLegend(p, legendX, legendY, cAi,
            timemaster::I18n::t("widget.source.ai_fmt").arg(int(rAi * 100)));
        legendX += 100;
    }
    if (m_import > 0)
        drawLegend(p, legendX, legendY, cImport,
            timemaster::I18n::t("widget.source.import_fmt").arg(int(rImport * 100)));
}

// 绘制单段：左段四角圆角，中/右段仅右侧圆角避免拼接缝隙
void SourceBar::drawSegment(QPainter &p, int x, int y, int w, int h, QColor c, bool left) {
    if (w <= 0) return;
    // FIX (V4.2): 原来的 path 用 OddEvenFill 默认规则，叠加同一个 addRoundedRect
    // 会让两条路径互相抵消，结果什么都不画——就是用户看到的"空白外框"。
    // 改成最直白的填充。
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    if (left) {
        // 最左段：四角都圆
        p.drawRoundedRect(QRectF(x, y, w, h), 6, 6);
    } else {
        // 中段 / 右段：左侧直边，右侧圆角
        QPainterPath path;
        path.addRoundedRect(QRectF(x - 6, y, w + 6, h), 6, 6);
        p.fillPath(path, c);
    }
}

// 绘制图例小色块 + 百分比文字
void SourceBar::drawLegend(QPainter &p, int x, int y, QColor c, const QString &txt) {
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawRect(QRect(x, y + 2, 10, 10));
    p.setPen(Theme::instance().textSecondary());
    QFont f = p.font();
    f.setPointSize(10);  // ~13px legend
    p.setFont(f);
    p.drawText(QRect(x + 14, y, 160, 18), Qt::AlignLeft | Qt::AlignVCenter, txt);
}

} // namespace timemaster
