#include "ui/settings/theme_manager.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>

namespace deadliner::ui
{

    namespace
    {

        // Shared structural stylesheet — palette() tokens auto-adapt to the active
        // QPalette, so this works for both light and dark modes without duplication.
        constexpr auto kBaseQss = R"(
/* ── Buttons ───────────────────────────────────────────────────────────────── */
QPushButton {
    border-radius: 6px;
    padding: 6px 20px;
    font-weight: 500;
    border: 1px solid palette(midlight);
    background-color: palette(button);
    color: palette(button-text);
    min-height: 28px;
}
QPushButton:hover {
    background-color: palette(light);
}
QPushButton:pressed {
    background-color: palette(mid);
}
QPushButton:disabled {
    color: palette(mid);
}

/* ── Sidebar navigation list ───────────────────────────────────────────────── */
QListWidget#navigationList {
    border: none;
    background: transparent;
    outline: none;
}
QListWidget#navigationList::item {
    padding: 10px 14px;
    border-radius: 8px;
    margin: 2px 4px;
    min-height: 20px;
}
QListWidget#navigationList::item:selected {
    background: palette(highlight);
    color: palette(highlighted-text);
}
QListWidget#navigationList::item:hover:!selected {
    background-color: rgba(128, 128, 128, 0.15);
}

/* ── Input fields ──────────────────────────────────────────────────────────── */
QLineEdit,
QSpinBox,
QDoubleSpinBox,
QDateTimeEdit,
QTimeEdit,
QComboBox {
    border: 1px solid palette(midlight);
    border-radius: 6px;
    padding: 4px 8px;
    padding-right: 28px;
    background: palette(base);
    color: palette(text);
    min-height: 26px;
}
QLineEdit:focus,
QSpinBox:focus,
QDoubleSpinBox:focus,
QDateTimeEdit:focus,
QTimeEdit:focus,
QComboBox:focus {
    border-color: palette(highlight);
}
QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 24px;
    border-top-right-radius: 6px;
    border-bottom-right-radius: 6px;
}
QDateTimeEdit::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 24px;
    border-top-right-radius: 6px;
    border-bottom-right-radius: 6px;
}
QComboBox::down-arrow,
QDateTimeEdit::down-arrow {
    width: 10px;
    height: 10px;
}
QComboBox QAbstractItemView {
    outline: none;
}
QSpinBox::up-button,
QSpinBox::down-button,
QDoubleSpinBox::up-button,
QDoubleSpinBox::down-button,
QDateTimeEdit::up-button,
QDateTimeEdit::down-button,
QTimeEdit::up-button,
QTimeEdit::down-button {
    border: none;
    width: 18px;
}

/* ── Tables ────────────────────────────────────────────────────────────────── */
QTableWidget {
    border: 1px solid palette(midlight);
    border-radius: 6px;
    gridline-color: palette(midlight);
}
QHeaderView::section {
    background: palette(button);
    border: none;
    border-bottom: 1px solid palette(midlight);
    padding: 4px 8px;
    font-weight: 600;
}

/* ── Checkboxes ────────────────────────────────────────────────────────────── */
QCheckBox {
    spacing: 8px;
}
QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border-radius: 4px;
    border: 1px solid palette(midlight);
    background: palette(base);
}
QCheckBox::indicator:checked {
    background: palette(highlight);
    border-color: palette(highlight);
}

/* ── Scroll bars ───────────────────────────────────────────────────────────── */
QScrollBar:vertical {
    width: 8px;
    background: transparent;
    margin: 0;
}
QScrollBar::handle:vertical {
    background: palette(midlight);
    border-radius: 4px;
    min-height: 24px;
}
QScrollBar::handle:vertical:hover {
    background: palette(mid);
}
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {
    height: 0;
}
QScrollBar:horizontal {
    height: 8px;
    background: transparent;
}
QScrollBar::handle:horizontal {
    background: palette(midlight);
    border-radius: 4px;
    min-width: 24px;
}
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal {
    width: 0;
}

/* ── Tooltips ──────────────────────────────────────────────────────────────── */
QToolTip {
    border: 1px solid palette(midlight);
    border-radius: 4px;
    padding: 4px 8px;
    background: palette(tool-tip-base);
    color: palette(tool-tip-text);
}
)";

        constexpr auto kLightExtraQss = R"(
QComboBox::drop-down,
QDateTimeEdit::drop-down {
    border-left: 1px solid #c9c9c9;
    background: #f3f3f3;
}
QComboBox QAbstractItemView {
    border: 1px solid #c9c9c9;
    background: #ffffff;
    color: #202124;
    selection-background-color: #4a90e2;
    selection-color: #ffffff;
    padding: 0;
}
QComboBox QAbstractItemView::item {
    min-height: 20px;
    padding: 4px 8px;
    border: none;
}
QComboBox QAbstractItemView::item:selected {
    background: #4a90e2;
    color: #ffffff;
}
QComboBox QAbstractItemView::item:hover:!selected {
    background: #e8f0fe;
    color: #202124;
}
)";

        // Extra overrides for dark mode where palette tokens alone are insufficient.
        constexpr auto kDarkExtraQss = R"(
QPushButton {
    border-color: rgba(255, 255, 255, 0.12);
}
QListWidget#navigationList::item:hover:!selected {
    background-color: rgba(255, 255, 255, 0.08);
}
QLineEdit,
QSpinBox,
QDoubleSpinBox,
QDateTimeEdit,
QTimeEdit,
QComboBox {
    border-color: rgba(255, 255, 255, 0.12);
}
QComboBox::drop-down,
QDateTimeEdit::drop-down {
    border-left: 1px solid rgba(255, 255, 255, 0.12);
    background: rgb(52, 52, 56);
}
QComboBox QAbstractItemView {
    border: 1px solid rgba(255, 255, 255, 0.12);
    background: rgb(37, 37, 40);
    color: rgb(240, 240, 240);
    selection-background-color: rgb(74, 144, 226);
    selection-color: rgb(255, 255, 255);
    padding: 0;
}
QComboBox QAbstractItemView::item {
    min-height: 20px;
    padding: 4px 8px;
    border: none;
}
QComboBox QAbstractItemView::item:selected {
    background: rgb(74, 144, 226);
    color: rgb(255, 255, 255);
}
QComboBox QAbstractItemView::item:hover:!selected {
    background: rgba(255, 255, 255, 0.08);
    color: rgb(240, 240, 240);
}
QTableWidget {
    border-color: rgba(255, 255, 255, 0.10);
}
QHeaderView::section {
    border-bottom-color: rgba(255, 255, 255, 0.10);
}
)";

        QPalette buildDarkPalette()
        {
            QPalette p;
            p.setColor(QPalette::Window, QColor(37, 37, 40));
            p.setColor(QPalette::WindowText, QColor(240, 240, 240));
            p.setColor(QPalette::Base, QColor(28, 28, 30));
            p.setColor(QPalette::AlternateBase, QColor(46, 46, 50));
            p.setColor(QPalette::ToolTipBase, QColor(50, 50, 55));
            p.setColor(QPalette::ToolTipText, QColor(245, 245, 245));
            p.setColor(QPalette::Text, QColor(240, 240, 240));
            p.setColor(QPalette::Button, QColor(52, 52, 56));
            p.setColor(QPalette::ButtonText, QColor(240, 240, 240));
            p.setColor(QPalette::BrightText, Qt::white);
            p.setColor(QPalette::Light, QColor(64, 64, 68));
            p.setColor(QPalette::Mid, QColor(44, 44, 48));
            p.setColor(QPalette::Midlight, QColor(60, 60, 64));
            p.setColor(QPalette::Dark, QColor(24, 24, 26));
            p.setColor(QPalette::Shadow, QColor(10, 10, 12));
            p.setColor(QPalette::Highlight, QColor(74, 144, 226));
            p.setColor(QPalette::HighlightedText, Qt::white);
            p.setColor(QPalette::Link, QColor(99, 179, 237));
            p.setColor(QPalette::LinkVisited, QColor(138, 120, 230));
            return p;
        }

    } // namespace

    void applyTheme(QApplication &application, const QString &theme)
    {
        application.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

        if (theme == QStringLiteral("dark"))
        {
            application.setPalette(buildDarkPalette());
            application.setStyleSheet(QString::fromUtf8(kBaseQss) + QString::fromUtf8(kDarkExtraQss));
        }
        else
        {
            // Light or system: reset to standard Fusion palette and apply base QSS.
            application.setPalette(application.style()->standardPalette());
            application.setStyleSheet(QString::fromUtf8(kBaseQss) + QString::fromUtf8(kLightExtraQss));
        }
    }

} // namespace deadliner::ui
