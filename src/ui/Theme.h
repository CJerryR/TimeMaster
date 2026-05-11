#pragma once

#include "../core/Types.h"
#include <QObject>
#include <QColor>
#include <QSettings>

namespace timemaster {

/**
 * 设计语言：「Warm Paper · 暖纸感」
 *  暖色调设计系统。
 *
 *  色彩定量规范
 *  ─────────────────────────────────────
 *  · 底色（浅色）：#F5F2ED   — 模拟自然纸张反射率，比纯白少约 15% 蓝光刺激
 *  · 底色（深色）：#262521   — 暖调深棕黑，避免冷感
 *  · 品牌色：    #D97757   — 中等饱和度橙褐，HSL(17°, 60%, 60%)
 *  · 文字主色（浅）：#1D1C16 — 黑里带暖棕
 *  · 文字主色（深）：#F0ECE0 — 暖白
 *  · 描边：rgba(60,50,40,0.10) — 极低对比纸纹质感
 *
 *  几何
 *  ─────────────────────────────────────
 *  · 8pt 网格基数
 *  · 大容器圆角：16-24px
 *  · 小组件圆角：8px
 */
class Theme : public QObject {
    Q_OBJECT
public:
    enum Mode { Light, Dark };

    static Theme &instance();

    Mode mode() const { return m_mode; }
    void setMode(Mode m);
    void toggle();

    // ---- 不透明色 ----
    QColor bgPage() const;
    QColor bgPageTop() const;
    QColor bgPageBottom() const;
    QColor bgContainer() const;
    QColor bgComponent() const;
    QColor bgHover() const;
    QColor stroke() const;
    QColor textPrimary() const;
    QColor textSecondary() const;
    QColor textPlaceholder() const;
    QColor brand() const;
    QColor brandLight() const;
    QColor accent() const;
    QColor todayHighlight() const;
    QColor nowLine() const;
    QColor success() const;
    QColor danger() const;

    // ---- 半透明色（QSS 用） ----
    QString cardBgRgba() const;
    QString cardBgHoverRgba() const;
    QString sidebarBgRgba() const;
    QString componentBgRgba() const;
    QString strokeRgba() const;
    QString shadowRgba() const;

    QString pageGradient() const;

    QHash<EventColor, ColorPalette> palette() const;

    QString globalStylesheet() const;

signals:
    void changed();

private:
    explicit Theme(QObject *parent = nullptr);
    Mode m_mode = Light;
    QSettings m_settings;
};

} // namespace timemaster
