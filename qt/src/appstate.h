#pragma once

#include <QObject>
#include <QStringList>

/*
 * Application settings and session state.
 * Defaults mirror src/lib/settings.js at commit 3673f5a:
 *   lang "en-us", fontSize 12, tabSize 2, softTab true, textWrap false,
 *   linenumbers true, quickTools 2, editorTheme one_dark, fullscreen false.
 */
class AppState : public QObject {
    Q_OBJECT

public:
    static AppState *instance();

    /* ---- app settings ---- */
    QString langCode() const;
    void setLangCode(const QString &code);

    int fontSize() const;
    void setFontSize(int size);

    int tabSize() const;
    void setTabSize(int size);

    bool softTab() const;
    void setSoftTab(bool soft);

    bool textWrap() const;
    void setTextWrap(bool wrap);

    bool lineNumbers() const;
    void setLineNumbers(bool show);

    bool quickToolsEnabled() const;
    void setQuickToolsEnabled(bool enabled);

    bool fullscreen() const;
    void setFullscreen(bool fullscreen);

    bool rememberFiles() const;
    void setRememberFiles(bool remember);

    bool rememberFolders() const;
    void setRememberFolders(bool remember);

    /* ---- session state ---- */
    QStringList folders() const;
    void setFolders(const QStringList &folders);

    QStringList openFiles() const;
    void setOpenFiles(const QStringList &files);

    QStringList pinnedFiles() const;
    void setPinnedFiles(const QStringList &files);

    QString lastFile() const;
    void setLastFile(const QString &path);

    QStringList recentFiles() const;
    void pushRecentFile(const QString &path);

    bool quickToolsSearchMode() const;
    void setQuickToolsSearchMode(bool searchMode);

signals:
    void settingsChanged();

private:
    explicit AppState(QObject *parent = nullptr);
    void emitChanged();
};
