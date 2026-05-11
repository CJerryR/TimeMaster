#pragma once

#include "../core/Types.h"
#include <QObject>
#include <QColor>
#include <QSettings>

namespace timemaster {

/**
 * 应用主题：light / dark
 * 单例，全局监听变更
 *
 * 设计语言：「premium glass」
 *  - 主背景：柔和的对角线渐变（避免单调）
 *  - 卡片/容器：rgba 半透明覆盖，让底层渐变隐隐透出，营造层次感
 *  - 边框：低透明度（约 8%），细而不死
 *  - 圆角：12-14px（更现代）
 *  - 强调色：保留品牌红，但配以更细腻的灰阶
 */
class Theme : public QObject {
    Q_OBJECT
public:
    enum Mode { Light, Dark };

    static Theme &instance();

    Mode mode() const { return m_mode; }
    void setMode(Mode m);
    void toggle();

    // ---- 不透明色（用于绘制） ----
    QColor bgPage() const;            // 整体页面纯色回退
    QColor bgPageTop() const;         // 渐变起点（左上）
    QColor bgPageBottom() const;      // 渐变终点（右下）
    QColor bgContainer() const;       // 卡片不透明色（用于绘制饼图等）
    QColor bgComponent() const;
    QColor bgHover() const;
    QColor stroke() const;
    QColor textPrimary() const;
    QColor textSecondary() const;
    QColor textPlaceholder() const;
    QColor brand() const;
    QColor brandLight() const;
    QColor accent() const;            // 辅助强调（靛蓝）
    QColor todayHighlight() const;
    QColor nowLine() const;
    QColor success() const;
    QColor danger() const;

    // ---- 半透明色（QSS 用，rgba 字符串） ----
    QString cardBgRgba() const;       // 卡片背景 rgba
    QString cardBgHoverRgba() const;
    QString sidebarBgRgba() const;
    QString componentBgRgba() const;
    QString strokeRgba() const;       // 边框 rgba
    QString shadowRgba() const;       // 用于伪阴影边框

    // 页面背景渐变（QSS / QPalette 用）
    QString pageGradient() const;

    QHash<EventColor, ColorPalette> palette() const;

    /// 全局基础样式：覆盖输入框/按钮等所有控件
    QString globalStylesheet() const;

signals:
    void changed();

private:
    explicit Theme(QObject *parent = nullptr);
    Mode m_mode = Light;
    QSettings m_settings;
};

} // namespace timemaster
