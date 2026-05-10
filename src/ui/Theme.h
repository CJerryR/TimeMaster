#pragma once

#include "../core/Types.h"
#include <QObject>
#include <QColor>
#include <QSettings>

namespace timeplan {

/**
 * 应用主题：light / dark
 * 单例，全局监听变更
 */
class Theme : public QObject {
    Q_OBJECT
public:
    enum Mode { Light, Dark };

    static Theme &instance();

    Mode mode() const { return m_mode; }
    void setMode(Mode m);
    void toggle();

    // 全局色调用接口
    QColor bgPage() const;        // 整体背景
    QColor bgContainer() const;   // 卡片/容器
    QColor bgComponent() const;   // 输入框/小组件
    QColor bgHover() const;       // 悬浮态
    QColor stroke() const;        // 边框/分隔线
    QColor textPrimary() const;
    QColor textSecondary() const;
    QColor textPlaceholder() const;
    QColor brand() const;         // 品牌色（红）
    QColor brandLight() const;    // 品牌色 12% alpha 用于选中态背景
    QColor accentBlue() const;
    QColor todayHighlight() const;
    QColor nowLine() const;       // 当前时间线
    QColor success() const;
    QColor danger() const;

    QHash<EventColor, ColorPalette> palette() const;

    QString globalStylesheet() const;

signals:
    void changed();

private:
    explicit Theme(QObject *parent = nullptr);
    Mode m_mode = Light;
    QSettings m_settings;
};

} // namespace timeplan
