#include "appstate.h"

#include <QSettings>

AppState *AppState::instance() {
    static AppState *app = new AppState();
    return app;
}

AppState::AppState(QObject *parent) : QObject(parent) {
}

void AppState::emitChanged() {
    emit settingsChanged();
}

QString AppState::langCode() const {
    return QSettings().value(QStringLiteral("lang"), QStringLiteral("en-us")).toString();
}

void AppState::setLangCode(const QString &code) {
    QSettings().setValue(QStringLiteral("lang"), code);
    emitChanged();
}

int AppState::fontSize() const {
    return QSettings().value(QStringLiteral("editor/fontSize"), 12).toInt();
}

void AppState::setFontSize(int size) {
    QSettings().setValue(QStringLiteral("editor/fontSize"), size);
    emitChanged();
}

int AppState::tabSize() const {
    return QSettings().value(QStringLiteral("editor/tabSize"), 2).toInt();
}

void AppState::setTabSize(int size) {
    QSettings().setValue(QStringLiteral("editor/tabSize"), size);
    emitChanged();
}

bool AppState::softTab() const {
    return QSettings().value(QStringLiteral("editor/softTab"), true).toBool();
}

void AppState::setSoftTab(bool soft) {
    QSettings().setValue(QStringLiteral("editor/softTab"), soft);
    emitChanged();
}

bool AppState::textWrap() const {
    return QSettings().value(QStringLiteral("editor/textWrap"), false).toBool();
}

void AppState::setTextWrap(bool wrap) {
    QSettings().setValue(QStringLiteral("editor/textWrap"), wrap);
    emitChanged();
}

bool AppState::lineNumbers() const {
    return QSettings().value(QStringLiteral("editor/lineNumbers"), true).toBool();
}

void AppState::setLineNumbers(bool show) {
    QSettings().setValue(QStringLiteral("editor/lineNumbers"), show);
    emitChanged();
}

bool AppState::quickToolsEnabled() const {
    return QSettings().value(QStringLiteral("quickTools/enabled"), true).toBool();
}

void AppState::setQuickToolsEnabled(bool enabled) {
    QSettings().setValue(QStringLiteral("quickTools/enabled"), enabled);
    emitChanged();
}

bool AppState::fullscreen() const {
    return QSettings().value(QStringLiteral("app/fullscreen"), false).toBool();
}

void AppState::setFullscreen(bool fullscreen) {
    QSettings().setValue(QStringLiteral("app/fullscreen"), fullscreen);
    emitChanged();
}

bool AppState::rememberFiles() const {
    return QSettings().value(QStringLiteral("app/rememberFiles"), true).toBool();
}

void AppState::setRememberFiles(bool remember) {
    QSettings().setValue(QStringLiteral("app/rememberFiles"), remember);
    emitChanged();
}

bool AppState::rememberFolders() const {
    return QSettings().value(QStringLiteral("app/rememberFolders"), true).toBool();
}

void AppState::setRememberFolders(bool remember) {
    QSettings().setValue(QStringLiteral("app/rememberFolders"), remember);
    emitChanged();
}

QStringList AppState::folders() const {
    return QSettings().value(QStringLiteral("session/folders")).toStringList();
}

void AppState::setFolders(const QStringList &folders) {
    QSettings().setValue(QStringLiteral("session/folders"), folders);
}

QStringList AppState::openFiles() const {
    return QSettings().value(QStringLiteral("session/openFiles")).toStringList();
}

void AppState::setOpenFiles(const QStringList &files) {
    QSettings().setValue(QStringLiteral("session/openFiles"), files);
}

QStringList AppState::pinnedFiles() const {
    return QSettings().value(QStringLiteral("session/pinnedFiles")).toStringList();
}

void AppState::setPinnedFiles(const QStringList &files) {
    QSettings().setValue(QStringLiteral("session/pinnedFiles"), files);
}

QString AppState::lastFile() const {
    return QSettings().value(QStringLiteral("session/lastFile")).toString();
}

void AppState::setLastFile(const QString &path) {
    QSettings().setValue(QStringLiteral("session/lastFile"), path);
}

QStringList AppState::recentFiles() const {
    return QSettings().value(QStringLiteral("session/recent")).toStringList();
}

void AppState::pushRecentFile(const QString &path) {
    if (path.isEmpty()) return;
    QStringList recent = recentFiles();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > 30) recent.removeLast();
    QSettings().setValue(QStringLiteral("session/recent"), recent);
}

bool AppState::quickToolsSearchMode() const {
    return QSettings().value(QStringLiteral("session/searchMode"), false).toBool();
}

void AppState::setQuickToolsSearchMode(bool searchMode) {
    QSettings().setValue(QStringLiteral("session/searchMode"), searchMode);
}
