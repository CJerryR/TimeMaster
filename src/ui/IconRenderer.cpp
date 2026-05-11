#include "IconRenderer.h"

#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QPen>
#include <QFont>
#include <QtMath>

namespace timemaster {

namespace {

// 在像素 px 大小的画布上绘制一个图标（前景色 fg）
void drawIcon(QPainter &p, IconRenderer::Icon which, const QColor &fg, int px) {
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(fg);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    auto setStroke = [&](double w) { pen.setWidthF(w); p.setPen(pen); p.setBrush(Qt::NoBrush); };
    auto setFill = [&] { p.setPen(Qt::NoPen); p.setBrush(fg); };
    double s = px / 24.0;  // 比例尺：以 24×24 设计

    switch (which) {
        // ============ 导航：日历（带顶部装订 + 当日点） ============
        case IconRenderer::NavCalendar: {
            setStroke(1.6 * s);
            // 外框
            QRectF box(3.5 * s, 5.5 * s, 17 * s, 15 * s);
            p.drawRoundedRect(box, 2.5 * s, 2.5 * s);
            // 表头分割
            p.drawLine(QPointF(3.5 * s, 9.5 * s), QPointF(20.5 * s, 9.5 * s));
            // 装订（两条小竖线）
            p.drawLine(QPointF(8 * s, 3.5 * s), QPointF(8 * s, 7.5 * s));
            p.drawLine(QPointF(16 * s, 3.5 * s), QPointF(16 * s, 7.5 * s));
            // 高亮日
            setFill();
            p.drawEllipse(QPointF(15.5 * s, 15.5 * s), 1.7 * s, 1.7 * s);
            break;
        }
        // ============ 导航：分析（三柱状） ============
        case IconRenderer::NavAnalytics: {
            setStroke(1.6 * s);
            // 坐标轴
            p.drawLine(QPointF(4 * s, 4 * s), QPointF(4 * s, 20 * s));
            p.drawLine(QPointF(4 * s, 20 * s), QPointF(21 * s, 20 * s));
            // 柱子
            QPen filledPen = pen; filledPen.setWidthF(2.4 * s);
            p.setPen(filledPen);
            p.drawLine(QPointF(8 * s, 16 * s), QPointF(8 * s, 19 * s));
            p.drawLine(QPointF(12.5 * s, 11 * s), QPointF(12.5 * s, 19 * s));
            p.drawLine(QPointF(17 * s, 7 * s), QPointF(17 * s, 19 * s));
            break;
        }
        // ============ 导航：对话（气泡） ============
        case IconRenderer::NavChat: {
            setStroke(1.6 * s);
            QPainterPath path;
            QRectF r(3.5 * s, 4.5 * s, 17 * s, 13 * s);
            path.addRoundedRect(r, 3 * s, 3 * s);
            // 三角尾
            QPolygonF tail;
            tail << QPointF(8 * s, 17.5 * s)
                 << QPointF(7 * s, 21 * s)
                 << QPointF(11.5 * s, 17.5 * s);
            path.addPolygon(tail);
            p.drawPath(path.simplified());
            // 三个点
            setFill();
            p.drawEllipse(QPointF(9 * s, 11 * s), 1.1 * s, 1.1 * s);
            p.drawEllipse(QPointF(12 * s, 11 * s), 1.1 * s, 1.1 * s);
            p.drawEllipse(QPointF(15 * s, 11 * s), 1.1 * s, 1.1 * s);
            break;
        }
        // ============ 设置（齿轮） ============
        case IconRenderer::Settings: {
            setStroke(1.5 * s);
            // 8 齿
            p.save();
            p.translate(12 * s, 12 * s);
            for (int i = 0; i < 8; ++i) {
                p.drawLine(QPointF(0, -10 * s), QPointF(0, -7.5 * s));
                p.rotate(45);
            }
            p.restore();
            // 外圆
            p.drawEllipse(QPointF(12 * s, 12 * s), 6.5 * s, 6.5 * s);
            // 内圆
            p.drawEllipse(QPointF(12 * s, 12 * s), 2.2 * s, 2.2 * s);
            break;
        }
        // ============ 月亮 ============
        case IconRenderer::ThemeMoon: {
            setStroke(1.6 * s);
            QPainterPath crescent;
            crescent.moveTo(QPointF(18 * s, 14.5 * s));
            crescent.arcTo(QRectF(3.5 * s, 3.5 * s, 17 * s, 17 * s), 90, -180);
            crescent.arcTo(QRectF(7 * s, 2.5 * s, 14 * s, 14 * s), -90, 180);
            crescent.closeSubpath();
            setFill();
            p.drawPath(crescent);
            break;
        }
        // ============ 太阳 ============
        case IconRenderer::ThemeSun: {
            setStroke(1.7 * s);
            // 中心圆
            p.drawEllipse(QPointF(12 * s, 12 * s), 4 * s, 4 * s);
            // 8 根光线
            p.save();
            p.translate(12 * s, 12 * s);
            for (int i = 0; i < 8; ++i) {
                p.drawLine(QPointF(0, -7.5 * s), QPointF(0, -10 * s));
                p.rotate(45);
            }
            p.restore();
            break;
        }
        // ============ 刷新（带头箭头的圆弧） ============
        case IconRenderer::Refresh: {
            setStroke(1.7 * s);
            QRectF arcR(4 * s, 4 * s, 16 * s, 16 * s);
            p.drawArc(arcR, 60 * 16, 270 * 16);
            // 顶部小箭头
            QPolygonF arrow;
            arrow << QPointF(18 * s, 5.5 * s)
                  << QPointF(20.5 * s, 8 * s)
                  << QPointF(16 * s, 9.5 * s);
            setFill();
            p.drawPolygon(arrow);
            break;
        }
        // ============ Sparkle（四角星） ============
        case IconRenderer::Sparkle: {
            setFill();
            QPainterPath path;
            const double cx = 12 * s, cy = 12 * s;
            const double R = 9 * s, r = 2.6 * s;
            for (int i = 0; i < 8; ++i) {
                double ang = i * M_PI / 4.0;
                double rad = (i % 2 == 0) ? R : r;
                double x = cx + rad * qCos(ang - M_PI_2);
                double y = cy + rad * qSin(ang - M_PI_2);
                if (i == 0) path.moveTo(x, y);
                else path.lineTo(x, y);
            }
            path.closeSubpath();
            p.drawPath(path);
            // 右上一颗小星
            p.drawEllipse(QPointF(19.5 * s, 5 * s), 1.2 * s, 1.2 * s);
            break;
        }
        // ============ History（带指针的时钟） ============
        case IconRenderer::History: {
            setStroke(1.7 * s);
            p.drawEllipse(QPointF(12 * s, 12 * s), 8 * s, 8 * s);
            // 指针：12 点和 4 点
            p.drawLine(QPointF(12 * s, 12 * s), QPointF(12 * s, 7 * s));
            p.drawLine(QPointF(12 * s, 12 * s), QPointF(15.5 * s, 14 * s));
            break;
        }
        // ============ 左右箭头（贴图风：圆角粗线 + 大三角尖） ============
        case IconRenderer::ArrowLeft:
        case IconRenderer::ArrowRight: {
            bool left = (which == IconRenderer::ArrowLeft);
            setFill();
            // 等腰三角形（指向方向）
            QPolygonF tri;
            if (left) {
                tri << QPointF(8 * s, 12 * s)
                    << QPointF(15 * s, 6 * s)
                    << QPointF(15 * s, 18 * s);
            } else {
                tri << QPointF(16 * s, 12 * s)
                    << QPointF(9 * s, 6 * s)
                    << QPointF(9 * s, 18 * s);
            }
            QPainterPath triPath;
            triPath.addPolygon(tri);
            triPath.closeSubpath();
            // 用粗笔描以获得圆角观感
            QPen rp(fg); rp.setWidthF(2.4 * s); rp.setJoinStyle(Qt::RoundJoin); rp.setCapStyle(Qt::RoundCap);
            p.setPen(rp); p.setBrush(fg);
            p.drawPath(triPath);
            break;
        }
        // ============ 编辑（钢笔） ============
        case IconRenderer::Edit: {
            setStroke(1.6 * s);
            QPolygonF nib;
            nib << QPointF(4 * s, 20 * s)
                << QPointF(8 * s, 19.5 * s)
                << QPointF(19 * s, 8 * s)
                << QPointF(16 * s, 5 * s)
                << QPointF(4.5 * s, 16 * s);
            QPainterPath path; path.addPolygon(nib); path.closeSubpath();
            p.drawPath(path);
            p.drawLine(QPointF(14 * s, 7 * s), QPointF(17 * s, 10 * s));
            break;
        }
        // ============ 删除（垃圾桶） ============
        case IconRenderer::Delete: {
            setStroke(1.6 * s);
            // 桶身
            QPainterPath path;
            path.moveTo(QPointF(6 * s, 8 * s));
            path.lineTo(QPointF(7 * s, 20 * s));
            path.lineTo(QPointF(17 * s, 20 * s));
            path.lineTo(QPointF(18 * s, 8 * s));
            p.drawPath(path);
            // 盖板
            p.drawLine(QPointF(4 * s, 8 * s), QPointF(20 * s, 8 * s));
            // 提手
            p.drawLine(QPointF(9.5 * s, 8 * s), QPointF(9.5 * s, 5.5 * s));
            p.drawLine(QPointF(9.5 * s, 5.5 * s), QPointF(14.5 * s, 5.5 * s));
            p.drawLine(QPointF(14.5 * s, 5.5 * s), QPointF(14.5 * s, 8 * s));
            // 两道纹
            p.drawLine(QPointF(10 * s, 11 * s), QPointF(10 * s, 17 * s));
            p.drawLine(QPointF(14 * s, 11 * s), QPointF(14 * s, 17 * s));
            break;
        }
        case IconRenderer::Check: {
            setStroke(2.0 * s);
            p.drawLine(QPointF(5 * s, 12.5 * s), QPointF(10 * s, 17.5 * s));
            p.drawLine(QPointF(10 * s, 17.5 * s), QPointF(19 * s, 7 * s));
            break;
        }
        case IconRenderer::Plus: {
            setStroke(2.0 * s);
            p.drawLine(QPointF(12 * s, 5 * s), QPointF(12 * s, 19 * s));
            p.drawLine(QPointF(5 * s, 12 * s), QPointF(19 * s, 12 * s));
            break;
        }
        case IconRenderer::SourceManual: {
            // 手 / 笔尖
            setStroke(1.6 * s);
            p.drawLine(QPointF(5 * s, 19 * s), QPointF(15 * s, 9 * s));
            p.drawLine(QPointF(15 * s, 9 * s), QPointF(19 * s, 13 * s));
            p.drawLine(QPointF(19 * s, 13 * s), QPointF(9 * s, 23 * s));
            p.drawLine(QPointF(5 * s, 19 * s), QPointF(9 * s, 23 * s));
            break;
        }
        case IconRenderer::SourceAi: {
            // 小芯片
            setStroke(1.6 * s);
            QRectF box(7 * s, 7 * s, 10 * s, 10 * s);
            p.drawRoundedRect(box, 1.5 * s, 1.5 * s);
            for (double x : {10.0, 12.0, 14.0}) {
                p.drawLine(QPointF(x * s, 4 * s), QPointF(x * s, 7 * s));
                p.drawLine(QPointF(x * s, 17 * s), QPointF(x * s, 20 * s));
                p.drawLine(QPointF(4 * s, x * s), QPointF(7 * s, x * s));
                p.drawLine(QPointF(17 * s, x * s), QPointF(20 * s, x * s));
            }
            break;
        }
        case IconRenderer::SourceChat: {
            // 双气泡
            setStroke(1.6 * s);
            QRectF r1(3 * s, 5 * s, 12 * s, 9 * s);
            p.drawRoundedRect(r1, 2 * s, 2 * s);
            QRectF r2(9 * s, 11 * s, 12 * s, 9 * s);
            p.drawRoundedRect(r2, 2 * s, 2 * s);
            break;
        }
        default: break;
    }
}

// ====================================================================
// App 主图标（9 版方案，每版自带色彩）
// 256×256 设计基准
// ====================================================================
void drawAppIcon(QPainter &p, IconRenderer::Icon which, int px) {
    p.setRenderHint(QPainter::Antialiasing, true);
    double s = px / 96.0;

    auto roundBg = [&](const QColor &c) {
        p.setPen(Qt::NoPen);
        p.setBrush(c);
        p.drawRoundedRect(QRectF(6 * s, 6 * s, 84 * s, 84 * s), 20 * s, 20 * s);
    };

    switch (which) {
    case IconRenderer::AppIcon01_TickRingT: {
        roundBg(QColor("#D97757"));
        // 8 条细刻度
        QPen tickPen(QColor(255, 255, 255, 115)); tickPen.setWidthF(1.5 * s); tickPen.setCapStyle(Qt::RoundCap);
        p.setPen(tickPen);
        p.drawLine(QPointF(48 * s, 14 * s), QPointF(48 * s, 20 * s));
        p.drawLine(QPointF(48 * s, 76 * s), QPointF(48 * s, 82 * s));
        p.drawLine(QPointF(14 * s, 48 * s), QPointF(20 * s, 48 * s));
        p.drawLine(QPointF(76 * s, 48 * s), QPointF(82 * s, 48 * s));
        p.drawLine(QPointF(22 * s, 22 * s), QPointF(26 * s, 26 * s));
        p.drawLine(QPointF(70 * s, 22 * s), QPointF(74 * s, 26 * s));
        p.drawLine(QPointF(22 * s, 74 * s), QPointF(26 * s, 70 * s));
        p.drawLine(QPointF(70 * s, 74 * s), QPointF(74 * s, 70 * s));
        // T 字母
        QPen tPen(Qt::white); tPen.setWidthF(6 * s); tPen.setCapStyle(Qt::RoundCap);
        p.setPen(tPen);
        p.drawLine(QPointF(28 * s, 36 * s), QPointF(68 * s, 36 * s));
        p.drawLine(QPointF(48 * s, 36 * s), QPointF(48 * s, 68 * s));
        break;
    }
    case IconRenderer::AppIcon02_Hourglass: {
        roundBg(QColor("#F5F2ED"));
        QPen dk(QColor("#3C2D1F")); dk.setWidthF(3 * s); dk.setCapStyle(Qt::RoundCap); dk.setJoinStyle(Qt::RoundJoin);
        p.setPen(dk);
        p.drawLine(QPointF(26 * s, 20 * s), QPointF(70 * s, 20 * s));
        p.drawLine(QPointF(26 * s, 76 * s), QPointF(70 * s, 76 * s));
        QPolygonF hg;
        hg << QPointF(28 * s, 22 * s) << QPointF(68 * s, 22 * s)
           << QPointF(52 * s, 48 * s) << QPointF(68 * s, 74 * s)
           << QPointF(28 * s, 74 * s) << QPointF(44 * s, 48 * s);
        QPainterPath path; path.addPolygon(hg); path.closeSubpath();
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
        // 沙粒
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#D97757"));
        QPolygonF top;
        top << QPointF(32 * s, 26 * s) << QPointF(64 * s, 26 * s)
            << QPointF(52 * s, 44 * s) << QPointF(44 * s, 44 * s);
        p.drawPolygon(top);
        QPolygonF bot;
        bot << QPointF(48 * s, 52 * s) << QPointF(56 * s, 70 * s) << QPointF(40 * s, 70 * s);
        p.drawPolygon(bot);
        break;
    }
    case IconRenderer::AppIcon03_NightClock: {
        roundBg(QColor("#262521"));
        QPen orange(QColor("#D97757")); orange.setWidthF(3 * s);
        p.setPen(orange); p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(48 * s, 48 * s), 30 * s, 30 * s);
        QPen hand(QColor("#F0ECE0")); hand.setWidthF(3.5 * s); hand.setCapStyle(Qt::RoundCap);
        p.setPen(hand);
        p.drawLine(QPointF(48 * s, 48 * s), QPointF(48 * s, 26 * s));
        hand.setWidthF(3 * s); p.setPen(hand);
        p.drawLine(QPointF(48 * s, 48 * s), QPointF(62 * s, 48 * s));
        p.setPen(Qt::NoPen); p.setBrush(QColor("#D97757"));
        p.drawEllipse(QPointF(48 * s, 48 * s), 3.5 * s, 3.5 * s);
        break;
    }
    case IconRenderer::AppIcon04_CalendarDot: {
        roundBg(QColor("#F4DDD0"));
        QPen dk(QColor("#3C2D1F")); dk.setWidthF(3 * s); dk.setCapStyle(Qt::RoundCap); dk.setJoinStyle(Qt::RoundJoin);
        p.setPen(dk); p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(22 * s, 26 * s, 52 * s, 48 * s), 6 * s, 6 * s);
        p.drawLine(QPointF(22 * s, 38 * s), QPointF(74 * s, 38 * s));
        p.drawLine(QPointF(32 * s, 20 * s), QPointF(32 * s, 32 * s));
        p.drawLine(QPointF(64 * s, 20 * s), QPointF(64 * s, 32 * s));
        p.setPen(Qt::NoPen); p.setBrush(QColor("#D97757"));
        p.drawEllipse(QPointF(60 * s, 58 * s), 9 * s, 9 * s);
        break;
    }
    case IconRenderer::AppIcon05_Concentric: {
        roundBg(QColor("#D97757"));
        QPen rp(QColor("#FBF9F5")); rp.setWidthF(2.5 * s);
        QColor c1 = rp.color(); c1.setAlphaF(0.5); rp.setColor(c1); p.setPen(rp); p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(48 * s, 48 * s), 30 * s, 30 * s);
        c1.setAlphaF(0.75); rp.setColor(c1); p.setPen(rp);
        p.drawEllipse(QPointF(48 * s, 48 * s), 20 * s, 20 * s);
        p.setPen(Qt::NoPen); p.setBrush(QColor("#FBF9F5"));
        p.drawEllipse(QPointF(48 * s, 48 * s), 10 * s, 10 * s);
        QPen tp(QColor("#FBF9F5")); tp.setWidthF(3 * s); tp.setCapStyle(Qt::RoundCap);
        p.setPen(tp);
        p.drawLine(QPointF(48 * s, 18 * s), QPointF(48 * s, 14 * s));
        break;
    }
    case IconRenderer::AppIcon06_SunMountain: {
        roundBg(QColor("#F5F2ED"));
        p.setPen(Qt::NoPen); p.setBrush(QColor("#D97757"));
        p.drawEllipse(QPointF(48 * s, 44 * s), 14 * s, 14 * s);
        QPolygonF mtn;
        mtn << QPointF(14 * s, 70 * s) << QPointF(34 * s, 50 * s)
            << QPointF(48 * s, 62 * s) << QPointF(62 * s, 44 * s)
            << QPointF(82 * s, 70 * s);
        p.setBrush(QColor("#3C2D1F"));
        p.drawPolygon(mtn);
        QPen line(QColor("#3C2D1F")); line.setWidthF(2.5 * s); line.setCapStyle(Qt::RoundCap);
        p.setPen(line);
        p.drawLine(QPointF(14 * s, 76 * s), QPointF(82 * s, 76 * s));
        break;
    }
    case IconRenderer::AppIcon07_Compass: {
        roundBg(QColor("#262521"));
        QPen rim(QColor("#B5AEA3")); rim.setWidthF(2 * s);
        p.setPen(rim); p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(48 * s, 48 * s), 32 * s, 32 * s);
        // 指针：上半橙 + 下半白
        QPolygonF top;
        top << QPointF(48 * s, 22 * s) << QPointF(52 * s, 48 * s) << QPointF(44 * s, 48 * s);
        QPolygonF bot;
        bot << QPointF(48 * s, 74 * s) << QPointF(52 * s, 48 * s) << QPointF(44 * s, 48 * s);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#D97757")); p.drawPolygon(top);
        p.setBrush(QColor("#F0ECE0")); p.drawPolygon(bot);
        p.setBrush(QColor("#D97757"));
        p.drawEllipse(QPointF(48 * s, 48 * s), 3 * s, 3 * s);
        break;
    }
    case IconRenderer::AppIcon08_TimeBand: {
        roundBg(QColor("#F5F2ED"));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#D97757"));
        p.drawRect(QRectF(22 * s, 30 * s, 52 * s, 20 * s));
        p.setBrush(QColor("#B85A3D"));
        p.drawRect(QRectF(22 * s, 50 * s, 52 * s, 10 * s));
        p.setBrush(QColor("#8B3F26"));
        p.drawRect(QRectF(22 * s, 60 * s, 52 * s, 10 * s));
        QPen sep(QColor("#F5F2ED")); sep.setWidthF(2 * s); QColor sc = sep.color(); sc.setAlphaF(0.6); sep.setColor(sc);
        p.setPen(sep);
        p.drawLine(QPointF(34 * s, 22 * s), QPointF(34 * s, 78 * s));
        p.drawLine(QPointF(62 * s, 22 * s), QPointF(62 * s, 78 * s));
        break;
    }
    case IconRenderer::AppIcon09_TimeSeal: {
        roundBg(QColor("#B8453E"));
        QPen rim(QColor("#FBE7DC")); rim.setWidthF(2 * s);
        p.setPen(rim); p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(18 * s, 18 * s, 60 * s, 60 * s), 6 * s, 6 * s);
        QFont f; f.setFamily("Microsoft YaHei UI"); f.setPointSizeF(36 * s);
        f.setWeight(QFont::Black);
        p.setFont(f);
        p.setPen(QColor("#FBE7DC"));
        p.drawText(QRectF(18 * s, 14 * s, 60 * s, 52 * s), Qt::AlignCenter, "时");
        p.drawLine(QPointF(22 * s, 68 * s), QPointF(74 * s, 68 * s));
        QFont fs; fs.setPointSizeF(6 * s); p.setFont(fs);
        p.drawText(QRectF(22 * s, 70 * s, 52 * s, 8 * s), Qt::AlignCenter, "TIME MASTER");
        break;
    }
    default: break;
    }
}

} // anonymous namespace

QPixmap IconRenderer::pixmap(Icon which, const QColor &fg, int px) {
    QPixmap pm(px, px);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    drawIcon(p, which, fg, px);
    p.end();
    return pm;
}

QIcon IconRenderer::icon(Icon which, const QColor &fg, int px) {
    return QIcon(pixmap(which, fg, px));
}

QPixmap IconRenderer::appIcon(Icon which, int px) {
    QPixmap pm(px, px);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    drawAppIcon(p, which, px);
    p.end();
    return pm;
}

IconRenderer::Icon IconRenderer::defaultAppIcon() {
    // 用户挑选完图标后，改这一行即可（01-09）
    return AppIcon04_CalendarDot;
}

} // namespace timemaster
