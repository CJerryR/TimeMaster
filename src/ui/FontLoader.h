#pragma once

#include <QString>
#include <QStringList>

namespace timemaster {

/**
 * 字体加载器
 *  · 优先从 Qt 资源 :/fonts/ 加载内嵌字体
 *  · 其次从 exe 同级 fonts/ 目录加载用户放置的 .ttf
 *  · 用 Inter + 思源黑体 作为开源替代方案
 *  · 兜底 Segoe UI Variable / Microsoft YaHei UI（Windows 自带）
 */
class FontLoader {
public:
    // 启动时调用一次：扫描所有可用字体并注册到 QFontDatabase
    static void initialize();

    // 全局基础字体族（自动构建中英文回退链）
    static QString primaryFamily();   // 主显示字体（如 Inter）
    static QString cjkFamily();       // 中文字体（如 Noto Sans CJK SC）
    static QString monoFamily();      // 等宽字体
    static QString familyChain();     // 完整 CSS family 串

    // 是否成功加载了内嵌/外部字体（用于设置提示）
    static bool customLoaded();
};

} // namespace timemaster
