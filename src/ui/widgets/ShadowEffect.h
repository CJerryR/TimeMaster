//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QGraphicsDropShadowEffect>
#include <QWidget>
#include <QColor>

namespace timemaster {

/**
 * V4 § 7.2: a single, taste-conservative drop shadow profile for cards.
 *  · Light theme: very soft warm-grey shadow (matches the paper aesthetic).
 *  · Dark theme:  near-imperceptible — just enough to lift cards from the bg.
 *
 * Single helper so we don't end up with 11 slightly different shadow
 * configurations scattered through the codebase.
 */
class ShadowEffect {
public:
    enum Strength { Subtle, Card, Floating };

    static void apply(QWidget *w, Strength s = Card, bool darkMode = false) {
        if (!w) return;

        // Avoid stacking duplicate shadow effects
        if (auto *existing = qobject_cast<QGraphicsDropShadowEffect*>(w->graphicsEffect())) {
            Q_UNUSED(existing);
            return;
        }

        auto *eff = new QGraphicsDropShadowEffect(w);
        int blur = 0, dy = 0, alpha = 0;
        switch (s) {
            case Subtle:   blur = 8;  dy = 2; alpha = darkMode ? 32 : 14; break;
            case Card:     blur = 18; dy = 4; alpha = darkMode ? 48 : 22; break;
            case Floating: blur = 32; dy = 8; alpha = darkMode ? 70 : 38; break;
        }
        eff->setBlurRadius(blur);
        eff->setOffset(0, dy);
        // Warm-grey tint in light, soft black in dark
        eff->setColor(darkMode ? QColor(0, 0, 0, alpha)
                                : QColor(60, 50, 40, alpha));
        w->setGraphicsEffect(eff);
    }

    static void remove(QWidget *w) {
        if (w && w->graphicsEffect()) {
            w->setGraphicsEffect(nullptr);
        }
    }
};

} // namespace timemaster
