//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QString>
#include <QStringList>

namespace timemaster {

/**
 * V4.2 字体加载器
 *  · 优先从 Qt 资源 :/fonts/ 与 assets/fonts/ 子目录加载内嵌字体
 *  · Latin / 数字优先：IBM Plex Serif（用户要求的"文学/出版风"质感）
 *  · 中文：保留 Noto / 思源 / 微软雅黑 / 苹方 回退链
 *  · Slogan / 大字标语：Smiley Sans Oblique（得意黑）
 */
class FontLoader {
public:
    static void initialize();

    static QString primaryFamily();   // 主显示字体（IBM Plex Serif）
    static QString cjkFamily();       // 中文字体
    static QString monoFamily();      // 等宽字体
    static QString familyChain();     // 完整 CSS family 串

    // KPI / 日期等大数字（tnum）。V4.2 默认就是 IBM Plex Serif Medium
    static QString numericFamily();

    // V4.2 §6 新增：slogan / 大字标语字体（Smiley Sans Oblique）
    static QString slogenFamily();

    // V4.2 §6 新增：纯 Latin 显示（用于 IBM Plex Serif 强制段，比如 KPI、日期数字）
    static QString serifFamily();

    static bool customLoaded();
};

} // namespace timemaster
