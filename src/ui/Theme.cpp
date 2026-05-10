#include "Theme.h"

namespace timeplan {

Theme &Theme::instance() {
    static Theme inst;
    return inst;
}

Theme::Theme(QObject *parent)
    : QObject(parent),
      m_settings("TimeplanCpp", "TimeplanCpp")
{
    QString saved = m_settings.value("theme", "light").toString();
    m_mode = (saved == "dark") ? Dark : Light;
}

void Theme::setMode(Mode m) {
    if (m == m_mode) return;
    m_mode = m;
    m_settings.setValue("theme", m == Dark ? "dark" : "light");
    emit changed();
}

void Theme::toggle() {
    setMode(m_mode == Light ? Dark : Light);
}

QColor Theme::bgPage() const {
    return m_mode == Light ? QColor("#fafafa") : QColor("#161618");
}

QColor Theme::bgContainer() const {
    return m_mode == Light ? QColor("#ffffff") : QColor("#1f1f23");
}

QColor Theme::bgComponent() const {
    return m_mode == Light ? QColor("#f5f5f5") : QColor("#27272b");
}

QColor Theme::bgHover() const {
    return m_mode == Light ? QColor("#eeeeee") : QColor("#2e2e33");
}

QColor Theme::stroke() const {
    return m_mode == Light ? QColor("#e5e5e5") : QColor("#36363c");
}

QColor Theme::textPrimary() const {
    return m_mode == Light ? QColor("#1a1a1d") : QColor("#f4f4f5");
}

QColor Theme::textSecondary() const {
    return m_mode == Light ? QColor("#52525b") : QColor("#a1a1aa");
}

QColor Theme::textPlaceholder() const {
    return m_mode == Light ? QColor("#a1a1aa") : QColor("#71717a");
}

QColor Theme::brand() const {
    return QColor("#ef4444"); // 苹果日历红
}

QColor Theme::brandLight() const {
    QColor c = brand();
    c.setAlphaF(0.12);
    return c;
}

QColor Theme::accentBlue() const {
    return QColor("#0066ff");
}

QColor Theme::todayHighlight() const {
    return m_mode == Light ? QColor("#fff1f1") : QColor("#3a1818");
}

QColor Theme::nowLine() const {
    return QColor("#ef4444");
}

QColor Theme::success() const {
    return QColor("#16a34a");
}

QColor Theme::danger() const {
    return QColor("#dc2626");
}

QHash<EventColor, ColorPalette> Theme::palette() const {
    return m_mode == Light ? eventColorsLight() : eventColorsDark();
}

QString Theme::globalStylesheet() const {
    QString page = bgPage().name();
    QString container = bgContainer().name();
    QString component = bgComponent().name();
    QString hover = bgHover().name();
    QString strokeC = stroke().name();
    QString text = textPrimary().name();
    QString text2 = textSecondary().name();
    QString placeholder = textPlaceholder().name();
    QString brandC = brand().name();

    return QString(R"(
        QWidget {
            background-color: %1;
            color: %6;
            font-family: -apple-system, "SF Pro Display", "PingFang SC", "Helvetica Neue",
                         "Microsoft YaHei", system-ui, sans-serif;
            font-size: 13px;
        }
        QFrame#cardFrame, QFrame#sidebarFrame, QFrame#headerFrame, QWidget#contentWidget,
        QFrame#dialogContent {
            background-color: %2;
        }
        QPushButton {
            background-color: %3;
            border: 1px solid %5;
            border-radius: 8px;
            padding: 6px 14px;
            color: %6;
        }
        QPushButton:hover {
            background-color: %4;
        }
        QPushButton:pressed {
            background-color: %5;
        }
        QPushButton[class="primary"] {
            background-color: %9;
            color: white;
            border: none;
            font-weight: 600;
        }
        QPushButton[class="primary"]:hover {
            background-color: #dc2626;
        }
        QPushButton[class="primary"]:disabled {
            background-color: %3;
            color: %8;
        }
        QPushButton[class="ghost"] {
            background-color: transparent;
            border: none;
            color: %7;
            padding: 6px 12px;
        }
        QPushButton[class="ghost"]:hover {
            background-color: %4;
            color: %6;
        }
        QPushButton[class="navItem"] {
            text-align: left;
            padding: 9px 14px;
            border: none;
            background-color: transparent;
            color: %7;
            border-radius: 8px;
        }
        QPushButton[class="navItem"]:hover {
            background-color: %4;
            color: %6;
        }
        QPushButton[class="navItem"]:checked,
        QPushButton[class="navItem"][active="true"] {
            background-color: rgba(239,68,68,0.12);
            color: %9;
            font-weight: 600;
        }
        QLineEdit, QTextEdit, QPlainTextEdit, QComboBox, QDateTimeEdit, QSpinBox {
            background-color: %3;
            border: 1px solid %5;
            border-radius: 8px;
            padding: 6px 10px;
            color: %6;
            selection-background-color: %9;
            selection-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus,
        QDateTimeEdit:focus, QSpinBox:focus {
            border: 1px solid %9;
            background-color: %2;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox QAbstractItemView {
            background-color: %2;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 4px;
            color: %6;
            selection-background-color: rgba(239,68,68,0.12);
            selection-color: %9;
            outline: 0;
        }
        QScrollBar:vertical {
            background-color: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background-color: %5;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %8;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QScrollBar:horizontal {
            background-color: transparent;
            height: 8px;
            margin: 0;
        }
        QScrollBar::handle:horizontal {
            background-color: %5;
            border-radius: 4px;
            min-width: 30px;
        }
        QToolTip {
            background-color: %2;
            color: %6;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 4px 8px;
        }
        QLabel[class="title"] {
            font-size: 18px;
            font-weight: 700;
            color: %6;
            background-color: transparent;
        }
        QLabel[class="subtitle"] {
            font-size: 14px;
            font-weight: 600;
            color: %6;
            background-color: transparent;
        }
        QLabel[class="caption"] {
            font-size: 11px;
            color: %7;
            background-color: transparent;
            margin-bottom: 4px;
        }
        QLabel {
            background-color: transparent;
        }
        QDialog {
            background-color: %2;
        }
        QCheckBox, QRadioButton {
            color: %6;
            background-color: transparent;
        }
        QCheckBox::indicator, QRadioButton::indicator {
            width: 16px;
            height: 16px;
        }
    )")
    .arg(page, container, component, hover, strokeC,
         text, text2, placeholder, brandC);
}

} // namespace timeplan
