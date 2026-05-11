#include "RhythmCardWidget.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <algorithm>

namespace timemaster {

RhythmCardWidget::RhythmCardWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
    setMinimumHeight(200);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(2);

    m_title = new QLabel("🌅  日节奏");
    QFont tf;
    tf.setPointSize(11);
    tf.setWeight(QFont::DemiBold);
    m_title->setFont(tf);
    m_title->setStyleSheet("background:transparent;");
    lay->addWidget(m_title);

    m_subtitle = new QLabel("暂无数据");
    QFont sf;
    sf.setPointSize(9);
    m_subtitle->setFont(sf);
    m_subtitle->setStyleSheet("background:transparent;");
    lay->addWidget(m_subtitle);

    lay->addStretch(1); // 把绘制区域留给 paintEvent

    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, [this]() { applyTheme(); update(); });
}

void RhythmCardWidget::setHourlyData(const QList<HourlyBucket> &buckets) {
    m_buckets = buckets;
    recomputeSubtitle();
    update();
}

void RhythmCardWidget::recomputeSubtitle() {
    if (m_buckets.isEmpty()) {
        m_subtitle->setText("暂无数据");
        return;
    }
    qint64 maxV = 0;
    int peakHour = -1;
    qint64 total = 0;
    for (const auto &b : m_buckets) {
        total += b.totalMinutes;
        if (b.totalMinutes > maxV) {
            maxV = b.totalMinutes;
            peakHour = b.hour;
        }
    }
    if (peakHour < 0 || maxV <= 0) {
        m_subtitle->setText("暂无活跃时段");
        return;
    }
    QString period;
    if (peakHour < 6) period = "凌晨";
    else if (peakHour < 12) period = "上午";
    else if (peakHour < 18) period = "下午";
    else period = "晚上";
    m_subtitle->setText(QString("最活跃 %1 %2 时 · 累计 %3 小时")
                            .arg(period)
                            .arg(peakHour, 2, 10, QChar('0'))
                            .arg(QString::number(total / 60.0, 'f', 1)));
}

void RhythmCardWidget::applyTheme() {
    auto &t = Theme::instance();
    m_title->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPrimary().name()));
    m_subtitle->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textSecondary().name()));
}

void RhythmCardWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto &theme = Theme::instance();

    // 留出顶部标题区域 (~ 46px)
    const int topReserve = 50;
    const int padL = 6;
    const int padR = 6;
    const int padB = 18; // 留 X 轴刻度位置
    const int areaY = topReserve;
    const int areaH = std::max(40, height() - topReserve - padB);
    const int areaW = std::max(24, width() - padL - padR);

    // 24 个柱
    const int slotCount = 24;
    const double slotW = double(areaW) / double(slotCount);
    const int barW = std::max(3, int(slotW * 0.55));

    qint64 maxV = 0;
    QHash<int, qint64> mp;
    for (const auto &b : m_buckets) {
        mp.insert(b.hour, b.totalMinutes);
        if (b.totalMinutes > maxV) maxV = b.totalMinutes;
    }

    if (maxV <= 0) {
        // 画轻量基线 + 占位文字
        p.setPen(theme.textPlaceholder());
        QRect r(0, areaY, width(), areaH);
        p.drawText(r, Qt::AlignCenter, "暂无数据");
        return;
    }

    QColor accent = theme.brand();
    QColor base = theme.bgHover();

    for (int h = 0; h < slotCount; ++h) {
        qint64 v = mp.value(h, 0);
        double ratio = double(v) / double(maxV);
        int bH = std::max(2, int(areaH * ratio));
        int cx = padL + int(slotW * h + slotW / 2);
        int bx = cx - barW / 2;
        int by = areaY + areaH - bH;

        // 基线条
        QRect baseRect(bx, areaY + areaH - 2, barW, 2);
        QPainterPath bp;
        bp.addRoundedRect(baseRect, 1, 1);
        p.fillPath(bp, base);

        // 实际条
        if (v > 0) {
            QRect br(bx, by, barW, bH);
            QPainterPath bpath;
            bpath.addRoundedRect(br, 3, 3);
            QLinearGradient g(br.topLeft(), br.bottomLeft());
            QColor c0 = accent.lighter(115);
            QColor c1 = accent;
            g.setColorAt(0.0, c0);
            g.setColorAt(1.0, c1);
            p.fillPath(bpath, g);
        }

        // X 轴刻度：每 4 小时一个
        if (h % 4 == 0) {
            p.setPen(theme.textPlaceholder());
            QFont f = font();
            f.setPointSize(8);
            p.setFont(f);
            QRect tr(cx - 16, areaY + areaH + 2, 32, 14);
            p.drawText(tr, Qt::AlignCenter, QString::number(h));
        }
    }
}

} // namespace timemaster
