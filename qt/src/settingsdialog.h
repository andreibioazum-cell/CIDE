#pragma once

#include <QDialog>
#include <QStringList>

#include "icons.h"

#include <functional>

class QCheckBox;
class QComboBox;
class QLabel;
class QSpinBox;
class QStackedWidget;

/*
 * Settings dialog styled after src/settings/mainSettings.js at 3673f5a:
 * a category list (App settings, Editor settings, Terminal settings, About)
 * with sub-pages.
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

signals:
    void languageChanged();

private:
    QWidget *createMainPage();
    QWidget *createAppSettingsPage();
    QWidget *createEditorSettingsPage();
    QWidget *createTerminalSettingsPage();
    QWidget *createAboutPage();
    QWidget *createRow(Icons::Icon icon, const QString &text, const QString &info, const std::function<void()> &onActivate);
    void openPage(int index);

    QStackedWidget *m_stack = nullptr;
    QComboBox *m_languageCombo = nullptr;
    QCheckBox *m_fullscreenCheck = nullptr;
    QCheckBox *m_quickToolsCheck = nullptr;
    QCheckBox *m_rememberFilesCheck = nullptr;
    QCheckBox *m_rememberFoldersCheck = nullptr;
    QSpinBox *m_fontSizeSpin = nullptr;
    QSpinBox *m_tabSizeSpin = nullptr;
    QCheckBox *m_softTabCheck = nullptr;
    QCheckBox *m_wrapCheck = nullptr;
    QCheckBox *m_lineNumbersCheck = nullptr;
    QStringList m_languageNames;
};
