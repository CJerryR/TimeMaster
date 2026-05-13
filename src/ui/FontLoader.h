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
// 字体加载器：从资源和文件系统扫描 .ttf/.otf，分类为主字体/CJK/等宽/Serif/Slogan 五档，提供 CSS font-family 链
class FontLoader {
public:
    // 初始化：扫描并注册字体，按优先级分类
    static void initialize();

    // 获取主显示字体名称
    static QString primaryFamily();   // 主显示字体（IBM Plex Serif）
    // 获取中文字体名称
    static QString cjkFamily();       // 中文字体
    // 获取等宽字体名称
    static QString monoFamily();      // 等宽字体
    // 获取完整 CSS font-family 回退链
    static QString familyChain();     // 完整 CSS family 串

    // 获取数字专用字体（KPI / 日期等大数字）
    // KPI / 日期等大数字（tnum）。V4.2 默认就是 IBM Plex Serif Medium
    static QString numericFamily();

    // 获取 Slogan / 大字标语字体
    // V4.2 §6 新增：slogan / 大字标语字体（Smiley Sans Oblique）
    static QString slogenFamily();

    // 获取 Serif 字体（纯 Latin 显示）
    // V4.2 §6 新增：纯 Latin 显示（用于 IBM Plex Serif 强制段，比如 KPI、日期数字）
    static QString serifFamily();

    // 是否已加载自定义字体文件
    static bool customLoaded();
};

} // namespace timemaster
