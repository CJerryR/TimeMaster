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
