//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QSize>

namespace timemaster {

/**
 * 自绘图标。所有图标用 QPainter 绘制，能跟随主题变色，
 * 不依赖任何外部 .png / .svg 文件，也不会变成模糊位图。
 */
// 图标渲染器：QPainter 矢量绘制全部图标（导航/工具/AI/箭头/编辑/应用图标），DPI 感知，无外部图片依赖
class IconRenderer {
public:
    // 全部图标枚举：导航 / 工具 / AI / 箭头 / 通用 / 来源 / 应用图标 9 版
    enum Icon {
        // 导航
        NavCalendar,
        NavAnalytics,
        NavChat,
        // 工具按钮
        Settings,
        ThemeMoon,
        ThemeSun,
        Refresh,
        // 输入条 / AI
        Sparkle,
        History,
        // 导航箭头
        ArrowLeft,
        ArrowRight,
        // 通用
        Edit,
        Delete,
        Check,
        Plus,
        // 来源（统计页）
        SourceManual,
        SourceAi,
        SourceChat,
        // App 主图标（9 版可选）
        AppIcon01_TickRingT,
        AppIcon02_Hourglass,
        AppIcon03_NightClock,
        AppIcon04_CalendarDot,
        AppIcon05_Concentric,
        AppIcon06_SunMountain,
        AppIcon07_Compass,
        AppIcon08_TimeBand,
        AppIcon09_TimeSeal,
    };

    // 生成单色 QIcon（矢量缩放，DPI 感知）
    static QIcon icon(Icon which, const QColor &fg, int px = 24);
    // 生成单色 QPixmap（矢量缩放，DPI 感知）
    static QPixmap pixmap(Icon which, const QColor &fg, int px = 24);

    // 生成多色 App 主图标（DPI 感知）
    // App 主图标（多色，不跟主题变色），用于左上角 logo 显示（DPI 感知）
    static QPixmap appIcon(Icon which, int px = 256);

    // 生成原生分辨率 App 主图标（不缩放）
    // App 主图标的原生分辨率版本，给 QApplication::setWindowIcon 喂多档位图用
    static QPixmap appIconRaw(Icon which, int px = 256);

    // 获取当前默认 App 图标编号
    // 当前默认 App 图标编号（用户挑选后改这里即可）
    static Icon defaultAppIcon();
};

} // namespace timemaster
