#include "theme.h"

#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>

namespace Theme {

void apply() {
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QPalette p;
    p.setColor(QPalette::Window, Primary);
    p.setColor(QPalette::WindowText, PrimaryText);
    p.setColor(QPalette::Base, Secondary);
    p.setColor(QPalette::AlternateBase, Primary);
    p.setColor(QPalette::Text, PrimaryText);
    p.setColor(QPalette::Button, Secondary);
    p.setColor(QPalette::ButtonText, PrimaryText);
    p.setColor(QPalette::Highlight, Active);
    p.setColor(QPalette::HighlightedText, ActiveText);
    p.setColor(QPalette::ToolTipBase, Primary);
    p.setColor(QPalette::ToolTipText, PrimaryText);
    p.setColor(QPalette::Link, Link);
    p.setColor(QPalette::PlaceholderText, QColor(255, 255, 255, 100));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(245, 245, 245, 100));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(245, 245, 245, 100));
    p.setColor(QPalette::Disabled, QPalette::Window, QColor(35, 39, 42));
    QApplication::setPalette(p);

    qApp->setStyleSheet(QStringLiteral(R"==CSS==(
        /* ---- context menus (components/contextmenu) ---- */
        QMenu {
            background-color: #232729;
            color: #f5f5f5;
            border: 1px solid rgba(188, 188, 188, 0.15);
            border-radius: 8px;
            padding: 6px 0;
        }
        QMenu::item {
            padding: 8px 26px 8px 16px;
            font-size: 14px;
            border-radius: 4px;
            margin: 0 6px;
        }
        QMenu::item:selected {
            background-color: #4285f4;
            color: #ffffff;
        }
        QMenu::item:disabled {
            color: rgba(245, 245, 245, 0.4);
        }
        QMenu::separator {
            height: 1px;
            background: rgba(188, 188, 188, 0.15);
            margin: 5px 10px;
        }
        QMenu::indicator {
            width: 16px;
            height: 16px;
        }

        /* ---- inputs ---- */
        QLineEdit {
            background-color: rgba(0, 0, 0, 0.2);
            color: #f5f5f5;
            border: 1px solid rgba(188, 188, 188, 0.15);
            border-radius: 4px;
            padding: 5px 8px;
            font-size: 14px;
            selection-background-color: #4285f4;
        }
        QLineEdit:focus {
            border: 1px solid #4285f4;
        }

        QPlainTextEdit, QTextEdit {
            background-color: #282c34;
            color: #abb2bf;
            border: none;
            selection-background-color: #3e4451;
            selection-color: #abb2bf;
        }

        /* ---- buttons ---- */
        QPushButton {
            background-color: #4285f4;
            color: #ffffff;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 14px;
            text-transform: uppercase;
        }
        QPushButton:pressed {
            background-color: #2c8ef0;
        }
        QPushButton:disabled {
            background-color: rgba(66, 133, 244, 0.4);
            color: rgba(255, 255, 255, 0.6);
        }

        /* ---- lists / trees (sidebar file tree, dialogs) ---- */
        QTreeWidget, QListWidget {
            background-color: #232729;
            color: #f5f5f5;
            border: none;
            font-size: 14px;
            outline: none;
        }
        QTreeWidget::item, QListWidget::item {
            height: 30px;
            border: none;
        }
        QTreeWidget::item:selected, QListWidget::item:selected {
            background-color: rgba(0, 0, 0, 0.2);
            color: #f5f5f5;
        }
        QTreeWidget::item:hover, QListWidget::item:hover {
            background-color: rgba(255, 255, 255, 0.06);
        }
        QTreeView::branch {
            background: transparent;
        }

        QHeaderView::section {
            background-color: #232729;
            color: #f5f5f5;
            border: none;
            padding: 4px;
        }

        /* ---- scrollbars (width: 4px, web var --scrollbar-width) ---- */
        QScrollBar:vertical {
            background: transparent;
            width: 4px;
            margin: 0;
        }
        QScrollBar:horizontal {
            background: transparent;
            height: 4px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(255, 255, 255, 0.2);
            border-radius: 2px;
            min-height: 24px;
        }
        QScrollBar::handle:horizontal {
            background: rgba(255, 255, 255, 0.2);
            border-radius: 2px;
            min-width: 24px;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            width: 0;
            height: 0;
        }
        QScrollBar::add-page, QScrollBar::sub-page {
            background: transparent;
        }

        /* ---- dialogs ---- */
        QDialog {
            background-color: #232729;
            color: #f5f5f5;
        }
        QMessageBox {
            background-color: #232729;
            color: #f5f5f5;
        }
        QMessageBox QLabel {
            color: #f5f5f5;
            font-size: 14px;
        }
        QInputDialog {
            background-color: #232729;
            color: #f5f5f5;
        }
        QInputDialog QLabel {
            color: #f5f5f5;
            font-size: 14px;
        }
        QComboBox {
            background-color: rgba(0, 0, 0, 0.2);
            color: #f5f5f5;
            border: 1px solid rgba(188, 188, 188, 0.15);
            border-radius: 4px;
            padding: 5px 28px 5px 8px;
            font-size: 14px;
        }
        QComboBox QAbstractItemView {
            background-color: #232729;
            color: #f5f5f5;
            selection-background-color: #4285f4;
        }
        QSpinBox {
            background-color: rgba(0, 0, 0, 0.2);
            color: #f5f5f5;
            border: 1px solid rgba(188, 188, 188, 0.15);
            border-radius: 4px;
            padding: 4px 6px;
            font-size: 14px;
        }

        /* ---- checkboxes (components/checkbox) ---- */
        QCheckBox {
            color: #f5f5f5;
            font-size: 14px;
            spacing: 10px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 4px;
            border: 2px solid rgba(188, 188, 188, 0.4);
            background: transparent;
        }
        QCheckBox::indicator:checked {
            background: #4285f4;
            border: 2px solid #4285f4;
        }

        QToolTip {
            background-color: #232729;
            color: #f5f5f5;
            border: 1px solid rgba(188, 188, 188, 0.15);
            padding: 4px 8px;
            font-size: 12px;
        }

        QScrollArea {
            border: none;
        }

        /* ---- shell surfaces (object-name rules don't propagate to children) ---- */
        #headerBar, #fileTabs, #quickTools, #sidebarContainer, #centralRoot {
            background-color: #232729;
        }
    )==CSS=="));
}

} // namespace Theme
