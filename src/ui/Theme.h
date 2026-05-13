//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QObject>
#include <QColor>
#include <QSettings>

namespace timemaster {

/**
 * Design language: "Warm Paper" (V4)
 *  Light: bgPage #F2EEE5, bgContainer #FFFFFF, brand #C26646.
 *  Dark : bgPage #26241F, bgContainer #2E2C26, brand #E08A6E.
 *  Geometry: cards 12px / buttons & inputs 8px / chips 6px.
 *  pageGradient() removed — pure bgPage().
 */
// 主题单例：「暖纸」设计语言，管理亮/暗双模式色板，发射 changed 信号驱动全 UI 热切换
class Theme : public QObject {
    Q_OBJECT
public:
    enum Mode { Light, Dark };

    // 返回单例引用
    static Theme &instance();

    // 返回当前模式
    Mode mode() const { return m_mode; }
    // 设置模式并持久化
    void setMode(Mode m);
    // 切换亮/暗模式
    void toggle();

    // ---- 不透明色 ----
    // 页面背景色
    QColor bgPage() const;
    // 容器/卡片背景色
    QColor bgContainer() const;
    // 组件背景色
    QColor bgComponent() const;
    // 悬停背景色
    QColor bgHover() const;
    // 边框/分割线色
    QColor stroke() const;
    // 主文字色
    QColor textPrimary() const;
    // 次要文字色
    QColor textSecondary() const;
    // 占位文字色
    QColor textPlaceholder() const;
    // 品牌色
    QColor brand() const;
    // 品牌色浅色
    QColor brandLight() const;
    // 强调色
    QColor accent() const;
    // 今日高亮色
    QColor todayHighlight() const;
    // 当前时间线色
    QColor nowLine() const;
    // 成功色
    QColor success() const;
    // 危险/错误色
    QColor danger() const;

    // ---- 半透明色（QSS 用） ----
    // 卡片背景半透明色（QSS 用）
    QString cardBgRgba() const;
    // 卡片悬停背景半透明色（QSS 用）
    QString cardBgHoverRgba() const;
    // 侧边栏背景半透明色（QSS 用）
    QString sidebarBgRgba() const;
    // 组件背景半透明色（QSS 用）
    QString componentBgRgba() const;
    // 边框半透明色（QSS 用）
    QString strokeRgba() const;
    // 阴影半透明色（QSS 用）
    QString shadowRgba() const;

    // 返回 12 色事件色板
    QHash<EventColor, ColorPalette> palette() const;

    // 生成完整 QSS 样式表：字号阶梯 26/17/15/14/13、按钮、输入框、滚动条、日历控件
    QString globalStylesheet() const;

signals:
    // 主题切换信号
    void changed();

private:
    // 构造函数：从持久化设置读取模式
    explicit Theme(QObject *parent = nullptr);
    Mode m_mode = Light;
    QSettings m_settings;
};

} // namespace timemaster
