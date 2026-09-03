#pragma once

#include <QMainWindow>

#include <QShortcut>

#include <functional>

#include "codeeditor.h"
#include "filetabs.h"
#include "headerbar.h"
#include "highlighter.h"
#include "quicktools.h"
#include "filetree.h"

class QStackedWidget;
class WelcomeTab;

/*
 * Main editor window: header (tile), open-file-list, sidebar + editor
 * area, quick tools footer — the layout of the web interface at commit
 * 3673f5a, implemented in pure Qt Widgets.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    /* used by the offscreen screenshot tool */
    Sidebar *sidebar() const { return m_sidebar; }
    int openFileIn(const QString &path) { return openFile(path); }
    void activateTab(int tabIndex) { setActiveTab(tabIndex); }
    void showSidebar();
    void hideSidebar();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct EditorSession {
        QString path;      /* empty for an unsaved new file */
        QString title;
        bool pinned = false;
        bool readOnly = false;
        QString encoding = QStringLiteral("UTF-8");
        bool crlf = false;
        bool isEditor = true;
        CodeEditor *editor = nullptr;
        Highlighter *highlighter = nullptr;
    };

    void buildUi();
    void connectQuickTools();
    void connectTabs();
    void positionSidebar();

    /* files */
    int openFile(const QString &path);
    void newFile();
    void saveSession(int sessionIndex, bool saveAs);
    void closeSessionAt(int sessionIndex);
    void closeOtherTabs(int sessionIndex, bool toRight);
    bool confirmUnsaved(int sessionIndex);
    void renameActiveFile();
    void showFileProperties();
    void selectSyntax();
    void selectEncoding();
    void selectEol();
    void formatActiveFile();
    void insertColor();

    /* tabs */
    void setActiveTab(int tabIndex);
    int activeSessionIndex() const;
    int sessionToTabIndex(int sessionIndex) const;
    int tabIndexToSession(int tabIndex) const;
    void rebuildTabs();

    /* header / state */
    void updateHeader();
    void updateQuickToolsState();

    /* menus */
    void showMainMenu();
    void showFileMenu();
    void showTabContextMenu(int tabIndex, const QPoint &globalPos);

    /* dialogs */
    void openFolderDialog();
    void openFileDialog();
    void findFileDialog();
    void recentDialog();
    void gotoLineDialog();
    void consoleDialog();
    void runningProcessesDialog();
    void settingsDialog();
    void aboutDialog();
    void helpPage();

    /* search */
    void searchNext(const QString &term, bool matchCase, bool backwards = false);
    void replaceNext(const QString &term, const QString &replacement, bool matchCase);
    void replaceAll(const QString &term, const QString &replacement, bool matchCase);
    void updateSearchStatus(int position, int total);

    /* session (save-state) */
    void saveState();
    void restoreState();

    /* misc */
    void showToast(const QString &message);
    void log(const QString &message);
    void retranslate();
    void applySettingsToEditors();
    CodeEditor *activeEditor() const;

    HeaderBar *m_header = nullptr;
    FileTabs *m_tabs = nullptr;
    Sidebar *m_sidebar = nullptr;
    QuickTools *m_quickTools = nullptr;
    QStackedWidget *m_editorStack = nullptr;
    WelcomeTab *m_welcome = nullptr;
    QWidget *m_central = nullptr;
    QWidget *m_sidebarMask = nullptr;
    QLabel *m_toast = nullptr;

    QList<EditorSession> m_sessions;
    int m_activeSession = -1; /* -1 = welcome tab */

    QString m_lastSearchTerm;
    bool m_lastMatchCase = false;
    QStringList m_consoleLog;
};
