#include "mainwindow.h"
#include "appstate.h"
#include "icons.h"
#include "lang.h"
#include "settingsdialog.h"
#include "theme.h"
#include "welcometab.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QResizeEvent>
#include <QSaveFile>
#include <QScrollBar>
#include <QShortcut>
#include <QStackedWidget>
#include <QStringConverter>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace {

constexpr int kWelcomeTabIndex = 0;

bool isRunnableFile(const QString &path) {
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QLatin1String("html") || suffix == QLatin1String("htm");
}

QString decodeBytes(const QByteArray &bytes, const QString &encoding) {
    if (encoding == QLatin1String("UTF-16LE")) return QStringDecoder(QStringConverter::Utf16LE)(bytes);
    if (encoding == QLatin1String("UTF-16BE")) return QStringDecoder(QStringConverter::Utf16BE)(bytes);
    if (encoding == QLatin1String("ISO-8859-1")) return QStringDecoder(QStringConverter::Latin1)(bytes);
    if (encoding == QLatin1String("System")) return QStringDecoder(QStringConverter::System)(bytes);
    return QStringDecoder(QStringConverter::Utf8)(bytes);
}

QByteArray encodeText(const QString &text, const QString &encoding) {
    if (encoding == QLatin1String("UTF-16LE")) return QStringEncoder(QStringConverter::Utf16LE)(text);
    if (encoding == QLatin1String("UTF-16BE")) return QStringEncoder(QStringConverter::Utf16BE)(text);
    if (encoding == QLatin1String("ISO-8859-1")) return QStringEncoder(QStringConverter::Latin1)(text);
    if (encoding == QLatin1String("System")) return QStringEncoder(QStringConverter::System)(text);
    return QStringEncoder(QStringConverter::Utf8)(text);
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("CIDE"));
    resize(420, 840);

    buildUi();
    connectQuickTools();
    connectTabs();
    restoreState();

    /* shortcuts (web default key bindings) */
    auto shortcut = [this](const QKeySequence &keys, const std::function<void()> &handler) {
        QShortcut *action = new QShortcut(keys, this);
        connect(action, &QShortcut::activated, this, handler);
    };
    shortcut(QKeySequence::New, [this] { newFile(); });
    shortcut(QKeySequence::Open, [this] { openFileDialog(); });
    shortcut(QKeySequence::Save, [this] { saveSession(activeSessionIndex(), false); });
    shortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), [this] { saveSession(activeSessionIndex(), true); });
    shortcut(QKeySequence::Find, [this] { m_quickTools->setSearchMode(true); });
    shortcut(QKeySequence(Qt::CTRL | Qt::Key_G), [this] { gotoLineDialog(); });
    shortcut(QKeySequence::Close, [this] { closeSessionAt(activeSessionIndex()); });
    shortcut(QKeySequence(Qt::CTRL | Qt::Key_E), [this] { showMainMenu(); });
    shortcut(QKeySequence::HelpContents, [this] { helpPage(); });
    shortcut(QKeySequence::SelectAll, [this] {
        if (activeEditor()) activeEditor()->selectAll();
    });

    connect(AppState::instance(), &AppState::settingsChanged, this, [this] {
        applySettingsToEditors();
        if (AppState::instance()->fullscreen()) showFullScreen();
        else showMaximized();
        m_quickTools->setVisible(AppState::instance()->quickToolsEnabled());
    });

    applySettingsToEditors();
    updateHeader();
    updateQuickToolsState();
    rebuildTabs();
}

void MainWindow::buildUi() {
    m_header = new HeaderBar(this);
    connect(m_header, &HeaderBar::navToggled, this, [this] {
        m_sidebar->setVisible(!m_sidebar->isVisible());
    });
    connect(m_header, &HeaderBar::menuToggled, this, [this] { showMainMenu(); });
    connect(m_header, &HeaderBar::fileMenuToggled, this, [this] { showFileMenu(); });
    connect(m_header, &HeaderBar::runClicked, this, [this] {
        const int index = activeSessionIndex();
        if (index < 0) return;
        const QString path = m_sessions[index].path;
        if (!path.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    m_tabs = new FileTabs(this);

    m_sidebar = new Sidebar(this);
    connect(m_sidebar, &Sidebar::fileActivated, this, [this](const QString &path) {
        openFile(path);
    });
    connect(m_sidebar, &Sidebar::openFolderRequested, this, [this] { openFolderDialog(); });
    connect(m_sidebar, &Sidebar::searchResultActivated, this, [this](const QString &path, int line) {
        openFile(path);
        if (CodeEditor *editor = activeEditor()) editor->gotoLine(line);
    });

    m_editorStack = new QStackedWidget(this);
    m_welcome = new WelcomeTab(m_editorStack);
    connect(m_welcome, &WelcomeTab::newFileRequested, this, [this] { newFile(); });
    connect(m_welcome, &WelcomeTab::openFileRequested, this, [this] { openFileDialog(); });
    connect(m_welcome, &WelcomeTab::openFolderRequested, this, [this] { openFolderDialog(); });
    connect(m_welcome, &WelcomeTab::openTerminalRequested, this, [this] { runningProcessesDialog(); });
    connect(m_welcome, &WelcomeTab::openRecentRequested, this, [this] { recentDialog(); });
    connect(m_welcome, &WelcomeTab::commandPaletteRequested, this, [this] { findFileDialog(); });
    connect(m_welcome, &WelcomeTab::openSettingsRequested, this, [this] { settingsDialog(); });
    connect(m_welcome, &WelcomeTab::explorePluginsRequested, this, [this] {
        m_sidebar->setActiveApp(Sidebar::ExtensionsApp);
        m_sidebar->show();
    });
    connect(m_welcome, &WelcomeTab::helpRequested, this, [this] { helpPage(); });
    connect(m_welcome, &WelcomeTab::aboutRequested, this, [this] { aboutDialog(); });
    m_editorStack->addWidget(m_welcome);

    m_bodyRow = new QWidget(this);
    QHBoxLayout *bodyLayout = new QHBoxLayout(m_bodyRow);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    bodyLayout->addWidget(m_sidebar);
    bodyLayout->addWidget(m_editorStack, 1);

    m_quickTools = new QuickTools(this);

    m_central = new QWidget(this);
    m_central->setStyleSheet(QStringLiteral("background-color: #232729;"));
    QVBoxLayout *centralLayout = new QVBoxLayout(m_central);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(m_header);
    centralLayout->addWidget(m_tabs);
    centralLayout->addWidget(m_bodyRow, 1);
    centralLayout->addWidget(m_quickTools);

    m_toast = new QLabel(m_central);
    m_toast->setObjectName(QStringLiteral("toast"));
    m_toast->setStyleSheet(QStringLiteral(
        "background-color: #232729;"
        "color: #f5f5f5;"
        "border: 1px solid rgba(188, 188, 188, 0.15);"
        "border-radius: 4px;"
        "padding: 8px 16px;"
        "font-size: 13px;"));
    m_toast->hide();

    setCentralWidget(m_central);

    if (width() < 720) m_sidebar->hide();
}

void MainWindow::connectQuickTools() {
    connect(m_quickTools, &QuickTools::saveRequested, this, [this] { saveSession(activeSessionIndex(), false); });
    connect(m_quickTools, &QuickTools::undoRequested, this, [this] {
        if (activeEditor()) activeEditor()->undo();
    });
    connect(m_quickTools, &QuickTools::redoRequested, this, [this] {
        if (activeEditor()) activeEditor()->redo();
    });
    connect(m_quickTools, &QuickTools::textInsertRequested, this, [this](const QString &text) {
        if (activeEditor()) activeEditor()->insertAtCursor(text);
    });
    connect(m_quickTools, &QuickTools::tabRequested, this, [this] {
        if (activeEditor()) activeEditor()->insertAtCursor(QStringLiteral("\t"));
    });
    connect(m_quickTools, &QuickTools::escRequested, this, [this] {
        if (activeEditor()) {
            QTextCursor cursor = activeEditor()->textCursor();
            cursor.clearSelection();
            activeEditor()->setTextCursor(cursor);
        }
    });
    connect(m_quickTools, &QuickTools::arrowRequested, this, [this](int key, bool shift, bool ctrl) {
        CodeEditor *editor = activeEditor();
        if (!editor) return;
        QTextCursor::MoveOperation operation = QTextCursor::NoMove;
        switch (key) {
        case Qt::Key_Up: operation = QTextCursor::Up; break;
        case Qt::Key_Down: operation = QTextCursor::Down; break;
        case Qt::Key_Left: operation = ctrl ? QTextCursor::WordLeft : QTextCursor::Left; break;
        case Qt::Key_Right: operation = ctrl ? QTextCursor::WordRight : QTextCursor::Right; break;
        default: return;
        }
        QTextCursor::MoveMode mode = shift ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(operation, mode);
        editor->setTextCursor(cursor);
        editor->ensureCursorVisible();
    });
    connect(m_quickTools, &QuickTools::moveLineUpRequested, this, [this] {
        if (activeEditor()) activeEditor()->moveCurrentLineUp();
    });
    connect(m_quickTools, &QuickTools::moveLineDownRequested, this, [this] {
        if (activeEditor()) activeEditor()->moveCurrentLineDown();
    });
    connect(m_quickTools, &QuickTools::copyLineUpRequested, this, [this] {
        if (activeEditor()) activeEditor()->copyLineUp();
    });
    connect(m_quickTools, &QuickTools::copyLineDownRequested, this, [this] {
        if (activeEditor()) activeEditor()->copyLineDown();
    });
    connect(m_quickTools, &QuickTools::pasteRequested, this, [this] {
        if (activeEditor()) activeEditor()->paste();
    });
    connect(m_quickTools, &QuickTools::selectAllRequested, this, [this] {
        if (activeEditor()) activeEditor()->selectAll();
    });
    connect(m_quickTools, &QuickTools::searchNextRequested, this, [this](const QString &term, bool matchCase) {
        searchNext(term, matchCase, false);
    });
    connect(m_quickTools, &QuickTools::searchPrevRequested, this, [this](const QString &term, bool matchCase) {
        searchNext(term, matchCase, true);
    });
    connect(m_quickTools, &QuickTools::replaceNextRequested, this,
            [this](const QString &term, const QString &replacement, bool matchCase) {
        replaceNext(term, replacement, matchCase);
    });
    connect(m_quickTools, &QuickTools::replaceAllRequested, this,
            [this](const QString &term, const QString &replacement, bool matchCase) {
        replaceAll(term, replacement, matchCase);
    });
    connect(m_quickTools, &QuickTools::modeChanged, this, [this] {
        updateQuickToolsState();
    });
    connect(m_quickTools, &QuickTools::searchToggled, this, [this] {
        m_lastSearchTerm.clear();
    });
}

void MainWindow::connectTabs() {
    connect(m_tabs, &FileTabs::tabClicked, this, [this](int index) {
        setActiveTab(index);
    });
    connect(m_tabs, &FileTabs::tabContextMenu, this, [this](int index, const QPoint &globalPos) {
        showTabContextMenu(index, globalPos);
    });
}

/* ------------------------------------------------------------------ */
/* Session helpers                                                     */
/* ------------------------------------------------------------------ */

CodeEditor *MainWindow::activeEditor() const {
    if (m_activeSession < 0 || m_activeSession >= m_sessions.size()) return nullptr;
    return m_sessions[m_activeSession].editor;
}

int MainWindow::activeSessionIndex() const {
    return m_activeSession;
}

int MainWindow::sessionToTabIndex(int sessionIndex) const {
    return sessionIndex + 1; /* tab 0 is the welcome tab */
}

int MainWindow::tabIndexToSession(int tabIndex) const {
    return tabIndex - 1;
}

int MainWindow::openFile(const QString &path) {
    const QFileInfo info(path);
    if (!info.isFile() || !info.isReadable()) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("unable to open file")));
        return -1;
    }

    for (int i = 0; i < m_sessions.size(); ++i) {
        if (m_sessions[i].path == info.absoluteFilePath()) {
            setActiveTab(sessionToTabIndex(i));
            return i;
        }
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("unable to open file")));
        return -1;
    }
    const QByteArray bytes = file.readAll();
    file.close();

    EditorSession session;
    session.path = info.absoluteFilePath();
    session.title = info.fileName();
    session.encoding = QStringLiteral("UTF-8");
    QString text = decodeBytes(bytes, session.encoding);
    session.crlf = text.contains(QStringLiteral("\r\n"));
    if (session.crlf) text.remove(QStringLiteral("\r"));

    session.editor = new CodeEditor(m_editorStack);
    session.editor->setPlainText(text);
    session.editor->document()->setModified(false);
    session.highlighter = new Highlighter(session.editor->document(), Highlighter::languageForFile(info.fileName()));

    connect(session.editor, &CodeEditor::modificationChanged, this, [this](bool) {
        rebuildTabs();
        updateQuickToolsState();
    });
    connect(session.editor, &CodeEditor::editorFocus, this, [this] { updateQuickToolsState(); });
    connect(session.editor, &QPlainTextEdit::undoAvailable, this, [this](bool) { updateQuickToolsState(); });
    connect(session.editor, &QPlainTextEdit::redoAvailable, this, [this](bool) { updateQuickToolsState(); });

    m_editorStack->addWidget(session.editor);
    m_sessions.append(session);

    AppState::instance()->pushRecentFile(info.absoluteFilePath());
    setActiveTab(sessionToTabIndex(m_sessions.size() - 1));
    log(QStringLiteral("Opened %1").arg(info.absoluteFilePath()));
    return m_sessions.size() - 1;
}

void MainWindow::newFile() {
    /* if a folder is open, create the file there like the web version */
    const QStringList folders = m_sidebar->folders();
    if (!folders.isEmpty()) {
        QInputDialog dialog(this);
        dialog.setWindowTitle(Lang::s(QStringLiteral("new file")));
        dialog.setLabelText(Lang::s(QStringLiteral("type filename")));
        dialog.setInputMode(QInputDialog::TextInput);
        if (dialog.exec() == QDialog::Accepted) {
            const QString name = dialog.textValue().trimmed();
            if (!name.isEmpty()) {
                const QString path = QDir(folders.first()).filePath(name);
                if (QFile::exists(path)) {
                    QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                                         Lang::s(QStringLiteral("file already exists")));
                    openFile(path);
                    return;
                }
                QFile file(path);
                if (file.open(QIODevice::WriteOnly)) {
                    file.close();
                    openFile(path);
                } else {
                    QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                                         Lang::s(QStringLiteral("unable to save file")));
                }
                return;
            }
        }
        return;
    }

    /* no storage: create an unsaved "new file" tab */
    EditorSession session;
    session.path = QString();
    session.title = QStringLiteral("new file");
    session.editor = new CodeEditor(m_editorStack);
    session.highlighter = new Highlighter(session.editor->document(), Highlighter::PlainText);
    connect(session.editor, &CodeEditor::modificationChanged, this, [this](bool) {
        rebuildTabs();
        updateQuickToolsState();
    });
    connect(session.editor, &CodeEditor::editorFocus, this, [this] { updateQuickToolsState(); });
    connect(session.editor, &QPlainTextEdit::undoAvailable, this, [this](bool) { updateQuickToolsState(); });
    connect(session.editor, &QPlainTextEdit::redoAvailable, this, [this](bool) { updateQuickToolsState(); });
    m_editorStack->addWidget(session.editor);
    m_sessions.append(session);
    setActiveTab(sessionToTabIndex(m_sessions.size() - 1));
}

void MainWindow::saveSession(int sessionIndex, bool saveAs) {
    if (sessionIndex < 0 || sessionIndex >= m_sessions.size()) return;
    EditorSession &session = m_sessions[sessionIndex];

    QString target = session.path;
    if (saveAs || target.isEmpty()) {
        const QString suggested = target.isEmpty()
            ? QDir::home().filePath(session.title)
            : target;
        target = QFileDialog::getSaveFileName(this, Lang::s(QStringLiteral("save file as")), suggested);
        if (target.isEmpty()) return;
    }

    QString text = session.editor->toPlainText();
    if (session.crlf) {
        text.replace(QLatin1Char('\n'), QStringLiteral("\r\n"));
    }
    const QByteArray bytes = encodeText(text, session.encoding);

    QSaveFile file(target);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("unable to save file")));
        return;
    }
    file.write(bytes);
    if (!file.commit()) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("unable to save file")));
        return;
    }

    session.path = target;
    session.title = QFileInfo(target).fileName();
    session.editor->document()->setModified(false);
    if (session.highlighter) {
        session.highlighter->setLanguage(Highlighter::languageForFile(session.title));
    }
    AppState::instance()->pushRecentFile(target);
    rebuildTabs();
    updateHeader();
    showToast(Lang::s(QStringLiteral("file saved")));
    log(QStringLiteral("Saved %1").arg(target));
}

bool MainWindow::confirmUnsaved(int sessionIndex) {
    if (sessionIndex < 0 || sessionIndex >= m_sessions.size()) return true;
    const EditorSession &session = m_sessions[sessionIndex];
    if (!session.editor->document()->isModified()) return true;
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        Lang::s(QStringLiteral("close file")),
        Lang::s(QStringLiteral("unsaved file")),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    return answer == QMessageBox::Yes;
}

void MainWindow::closeSessionAt(int sessionIndex) {
    if (sessionIndex < 0 || sessionIndex >= m_sessions.size()) return;
    if (!confirmUnsaved(sessionIndex)) return;

    EditorSession session = m_sessions.takeAt(sessionIndex);
    m_editorStack->removeWidget(session.editor);
    delete session.editor;

    if (m_activeSession == sessionIndex) {
        m_activeSession = qBound(-1, sessionIndex - 1, m_sessions.size() - 1);
    } else if (m_activeSession > sessionIndex) {
        --m_activeSession;
    }

    if (m_activeSession >= 0) setActiveTab(sessionToTabIndex(m_activeSession));
    else setActiveTab(kWelcomeTabIndex);
    rebuildTabs();
    saveState();
}

void MainWindow::closeOtherTabs(int sessionIndex, bool toRight) {
    for (int i = m_sessions.size() - 1; i >= 0; --i) {
        if (i == sessionIndex) continue;
        if (toRight && i < sessionIndex) continue;
        if (!toRight && i > sessionIndex) continue;
        if (m_sessions[i].pinned) continue;
        if (m_sessions[i].editor->document()->isModified()) continue;
        EditorSession session = m_sessions.takeAt(i);
        m_editorStack->removeWidget(session.editor);
        delete session.editor;
        if (m_activeSession > i) --m_activeSession;
        else if (m_activeSession == i) m_activeSession = -1;
    }
    const int active = m_activeSession >= 0 ? m_activeSession : sessionIndex;
    m_activeSession = -1;
    if (active >= 0 && active < m_sessions.size()) setActiveTab(sessionToTabIndex(active));
    else setActiveTab(kWelcomeTabIndex);
    rebuildTabs();
    saveState();
}

/* ------------------------------------------------------------------ */
/* Tabs                                                                */
/* ------------------------------------------------------------------ */

void MainWindow::setActiveTab(int tabIndex) {
    if (tabIndex == kWelcomeTabIndex) {
        m_activeSession = -1;
        m_editorStack->setCurrentWidget(m_welcome);
    } else {
        const int sessionIndex = tabIndexToSession(tabIndex);
        if (sessionIndex < 0 || sessionIndex >= m_sessions.size()) return;
        m_activeSession = sessionIndex;
        m_editorStack->setCurrentWidget(m_sessions[sessionIndex].editor);
        m_sessions[sessionIndex].editor->setFocus();
    }
    m_tabs->setActiveIndex(tabIndex);
    m_tabs->ensureVisible(tabIndex);
    updateHeader();
    updateQuickToolsState();
}

void MainWindow::rebuildTabs() {
    QList<FileTabData> tabs;

    FileTabData welcome;
    welcome.id = QStringLiteral("welcome-tab");
    welcome.title = QStringLiteral("Get Started");
    welcome.isWelcome = true;
    welcome.closable = false;
    tabs.append(welcome);

    for (int i = 0; i < m_sessions.size(); ++i) {
        const EditorSession &session = m_sessions[i];
        FileTabData data;
        data.id = session.path.isEmpty()
            ? QStringLiteral("untitled://%1").arg(i)
            : session.path;
        data.title = session.title;
        data.fileName = session.title;
        data.unsaved = session.editor->document()->isModified();
        data.pinned = session.pinned;
        tabs.append(data);
    }

    const int activeTab = m_activeSession < 0 ? kWelcomeTabIndex : sessionToTabIndex(m_activeSession);
    m_tabs->setTabs(tabs);
    m_tabs->setActiveIndex(activeTab);
}

void MainWindow::updateHeader() {
    if (m_activeSession < 0) {
        m_header->setTitle(QStringLiteral("Get Started"));
        m_header->setSubTitle(QString());
        m_header->setFileMenuVisible(false);
        m_header->setRunVisible(false);
        return;
    }
    const EditorSession &session = m_sessions[m_activeSession];
    m_header->setTitle(session.title);
    m_header->setSubTitle(session.path.isEmpty()
        ? QString()
        : QFileInfo(session.path).absolutePath());
    m_header->setFileMenuVisible(true);
    m_header->setRunVisible(isRunnableFile(session.path));
}

void MainWindow::updateQuickToolsState() {
    CodeEditor *editor = activeEditor();
    m_quickTools->setSaveBadge(editor && editor->document()->isModified());
    m_quickTools->setCanUndo(editor && editor->document()->isUndoAvailable());
    m_quickTools->setCanRedo(editor && editor->document()->isRedoAvailable());
}

/* ------------------------------------------------------------------ */
/* Menus                                                               */
/* ------------------------------------------------------------------ */

void MainWindow::showMainMenu() {
    QMenu menu(this);

    const bool canSave = m_activeSession >= 0;

    auto item = [&menu](Icons::Icon icon, const QString &text, bool enabled, const std::function<void()> &handler) {
        QAction *action = menu.addAction(Icons::icon(icon), text);
        action->setEnabled(enabled);
        if (enabled) QObject::connect(action, &QAction::triggered, &menu, handler);
    };

    item(Icons::Add, Lang::s(QStringLiteral("new file")), true, [this] { newFile(); });
    item(Icons::Save, Lang::s(QStringLiteral("save")), canSave, [this] { saveSession(activeSessionIndex(), false); });
    item(Icons::Save, Lang::s(QStringLiteral("save as")), canSave, [this] { saveSession(activeSessionIndex(), true); });
    item(Icons::Folder, Lang::s(QStringLiteral("files")), true, [this] {
        m_sidebar->setActiveApp(Sidebar::FilesApp);
        m_sidebar->show();
    });
    item(Icons::Close, Lang::s(QStringLiteral("close file")), canSave, [this] { closeSessionAt(activeSessionIndex()); });
    item(Icons::History, Lang::s(QStringLiteral("open recent")), !AppState::instance()->recentFiles().isEmpty(), [this] { recentDialog(); });
    item(Icons::Search, Lang::s(QStringLiteral("find file")), true, [this] { findFileDialog(); });
    item(Icons::Code, Lang::s(QStringLiteral("console")), true, [this] { consoleDialog(); });
    item(Icons::Terminal, Lang::s(QStringLiteral("terminal")), true, [this] { runningProcessesDialog(); });
    menu.addSeparator();
    item(Icons::Settings, Lang::s(QStringLiteral("settings")), true, [this] { settingsDialog(); });
    item(Icons::Help, Lang::s(QStringLiteral("help")), true, [this] { helpPage(); });
    menu.addSeparator();
    item(Icons::Exit, Lang::s(QStringLiteral("exit")), true, [this] { close(); });

    const QPoint anchor(m_header->mapTo(m_central, QPoint(m_header->width() - 6, m_header->height())));
    menu.exec(m_central->mapToGlobal(anchor));
}

void MainWindow::showFileMenu() {
    const int sessionIndex = activeSessionIndex();
    if (sessionIndex < 0) return;
    const EditorSession &session = m_sessions[sessionIndex];
    const bool onDisk = !session.path.isEmpty();

    QMenu menu(this);

    auto item = [&menu](Icons::Icon icon, const QString &text, bool enabled, const std::function<void()> &handler) {
        /* text-only rows get a transparent icon so QMenu keeps alignment */
        QIcon menuIcon;
        if (icon != Icons::FileGlyph) {
            menuIcon = Icons::icon(icon);
        } else {
            QPixmap transparent(24, 24);
            transparent.fill(Qt::transparent);
            menuIcon = QIcon(transparent);
        }
        QAction *action = menu.addAction(menuIcon, text);
        action->setEnabled(enabled);
        if (enabled) QObject::connect(action, &QAction::triggered, &menu, handler);
    };

    item(Icons::Info, Lang::s(QStringLiteral("file properties")), onDisk, [this] { showFileProperties(); });
    menu.addSeparator();
    item(Icons::FileGlyph, Lang::s(QStringLiteral("rename")), true, [this] { renameActiveFile(); });
    item(Icons::FileGlyph, Lang::s(QStringLiteral("syntax highlighting")), true, [this] { selectSyntax(); });
    item(Icons::FileGlyph, Lang::s(QStringLiteral("encoding")) + QStringLiteral(": %1").arg(session.encoding), onDisk, [this] { selectEncoding(); });
    item(Icons::FileGlyph, Lang::s(QStringLiteral("new line mode")) + QStringLiteral(": %1").arg(session.crlf ? QStringLiteral("CRLF") : QStringLiteral("LF")), true, [this] { selectEol(); });

    QAction *readOnlyAction = menu.addAction(Lang::s(QStringLiteral("read only")));
    readOnlyAction->setCheckable(true);
    readOnlyAction->setChecked(session.readOnly);
    connect(readOnlyAction, &QAction::toggled, this, [this](bool checked) {
        if (CodeEditor *editor = activeEditor()) {
            editor->setReadOnly(checked);
            const int index = activeSessionIndex();
            if (index >= 0) m_sessions[index].readOnly = checked;
        }
    });

    item(Icons::FileGlyph, Lang::s(QStringLiteral("format")), true, [this] { formatActiveFile(); });
    menu.addSeparator();
    item(Icons::Share, Lang::s(QStringLiteral("share")), onDisk, [this] {
        const int index = activeSessionIndex();
        if (index < 0) return;
        QApplication::clipboard()->setText(m_sessions[index].path);
        showToast(m_sessions[index].path);
    });
    item(Icons::OpenInBrowser, Lang::s(QStringLiteral("open with")), onDisk, [this] {
        const int index = activeSessionIndex();
        if (index < 0) return;
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_sessions[index].path));
    });
    {
        QAction *editWith = menu.addAction(Lang::s(QStringLiteral("edit with")));
        editWith->setEnabled(false);
        QAction *homeScreen = menu.addAction(Lang::s(QStringLiteral("add to home screen")));
        homeScreen->setEnabled(false);
    }
    item(Icons::Pin, Lang::s(session.pinned ? QStringLiteral("unpin tab") : QStringLiteral("pin tab")), true, [this] {
        const int index = activeSessionIndex();
        if (index < 0) return;
        m_sessions[index].pinned = !m_sessions[index].pinned;
        rebuildTabs();
    });
    item(Icons::LastPage, Lang::s(QStringLiteral("close tabs to right")), true, [this] { closeOtherTabs(activeSessionIndex(), true); });
    item(Icons::FirstPage, Lang::s(QStringLiteral("close tabs to left")), true, [this] { closeOtherTabs(activeSessionIndex(), false); });
    item(Icons::CompareArrows, Lang::s(QStringLiteral("close other tabs")), true, [this] {
        const int index = activeSessionIndex();
        closeOtherTabs(index, false);
        closeOtherTabs(activeSessionIndex(), true);
    });
    menu.addSeparator();
    item(Icons::Search, Lang::s(QStringLiteral("search")), true, [this] { m_quickTools->setSearchMode(true); });
    item(Icons::SubdirArrowLeft, Lang::s(QStringLiteral("goto")), true, [this] { gotoLineDialog(); });
    menu.addSeparator();
    item(Icons::Palette, Lang::s(QStringLiteral("insert color")), true, [this] { insertColor(); });
    menu.addSeparator();
    {
        QAction *cut = menu.addAction(Lang::s(QStringLiteral("cut")));
        cut->setEnabled(activeEditor() && activeEditor()->textCursor().hasSelection());
        connect(cut, &QAction::triggered, this, [this] { if (activeEditor()) activeEditor()->cut(); });
        QAction *paste = menu.addAction(Lang::s(QStringLiteral("paste")));
        paste->setEnabled(activeEditor() != nullptr);
        connect(paste, &QAction::triggered, this, [this] { if (activeEditor()) activeEditor()->paste(); });
        QAction *copy = menu.addAction(Lang::s(QStringLiteral("copy")));
        copy->setEnabled(activeEditor() && activeEditor()->textCursor().hasSelection());
        connect(copy, &QAction::triggered, this, [this] { if (activeEditor()) activeEditor()->copy(); });
        QAction *selectAll = menu.addAction(Lang::s(QStringLiteral("select all")));
        selectAll->setEnabled(activeEditor() != nullptr);
        connect(selectAll, &QAction::triggered, this, [this] { if (activeEditor()) activeEditor()->selectAll(); });
    }

    const QPoint anchor(m_header->mapTo(m_central, QPoint(m_header->width() - 50, m_header->height())));
    menu.exec(m_central->mapToGlobal(anchor));
}

void MainWindow::showTabContextMenu(int tabIndex, const QPoint &globalPos) {
    QMenu menu(this);
    const int sessionIndex = tabIndexToSession(tabIndex);
    if (sessionIndex < 0 || sessionIndex >= m_sessions.size()) return;

    QAction *pin = menu.addAction(Lang::s(m_sessions[sessionIndex].pinned
        ? QStringLiteral("unpin tab") : QStringLiteral("pin tab")));
    connect(pin, &QAction::triggered, this, [this, sessionIndex] {
        if (sessionIndex >= 0 && sessionIndex < m_sessions.size()) {
            m_sessions[sessionIndex].pinned = !m_sessions[sessionIndex].pinned;
            rebuildTabs();
        }
    });
    QAction *close = menu.addAction(Lang::s(QStringLiteral("close file")));
    connect(close, &QAction::triggered, this, [this, sessionIndex] {
        closeSessionAt(sessionIndex);
    });
    menu.exec(globalPos);
}

/* ------------------------------------------------------------------ */
/* File actions                                                        */
/* ------------------------------------------------------------------ */

void MainWindow::renameActiveFile() {
    const int index = activeSessionIndex();
    if (index < 0) return;
    EditorSession &session = m_sessions[index];

    QInputDialog dialog(this);
    dialog.setWindowTitle(Lang::s(QStringLiteral("rename")));
    dialog.setLabelText(Lang::s(QStringLiteral("enter new name")));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextValue(session.title);
    if (dialog.exec() != QDialog::Accepted) return;
    const QString newName = dialog.textValue().trimmed();
    if (newName.isEmpty() || newName == session.title) return;

    if (!session.path.isEmpty()) {
        const QFileInfo info(session.path);
        const QString target = QDir(info.absolutePath()).filePath(newName);
        QFile file(session.path);
        if (!file.rename(target)) {
            QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                                 Lang::s(QStringLiteral("unable to rename")));
            return;
        }
        session.path = target;
        session.highlighter->setLanguage(Highlighter::languageForFile(newName));
    }
    session.title = newName;
    rebuildTabs();
    updateHeader();
    showToast(Lang::s(QStringLiteral("file renamed")));
}

void MainWindow::showFileProperties() {
    const int index = activeSessionIndex();
    if (index < 0) return;
    const EditorSession &session = m_sessions[index];
    const QFileInfo info(session.path);

    const int lines = session.editor->document()->blockCount();
    QString text = QStringLiteral(
        "%1: %2\n%3: %4\n%5: %6\n%7: %8\n%9: %10\n%11: %12\n%13: %14")
        .arg(Lang::s(QStringLiteral("path")), info.absoluteFilePath(),
             Lang::s(QStringLiteral("type")), Lang::s(QStringLiteral("file")),
             Lang::s(QStringLiteral("size")), QString::number(info.size()),
             Lang::s(QStringLiteral("line count")), QString::number(lines),
             Lang::s(QStringLiteral("encoding")), session.encoding,
             Lang::s(QStringLiteral("new line mode")), session.crlf ? QStringLiteral("CRLF") : QStringLiteral("LF"),
             Lang::s(QStringLiteral("syntax highlighting")),
             session.highlighter ? Highlighter::languageName(Highlighter::languageForFile(session.title)) : QString());

    QMessageBox box(this);
    box.setWindowTitle(Lang::s(QStringLiteral("file properties")));
    box.setText(text);
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}

void MainWindow::selectSyntax() {
    const int index = activeSessionIndex();
    if (index < 0) return;
    const QStringList names = Highlighter::languageNames();
    const QString current = Highlighter::languageName(Highlighter::languageForFile(m_sessions[index].title));

    bool ok = false;
    const QString selected = QInputDialog::getItem(this,
        Lang::s(QStringLiteral("syntax highlighting")),
        Lang::s(QStringLiteral("syntax highlighting")),
        names, names.indexOf(current), false, &ok);
    if (!ok || selected.isEmpty()) return;
    m_sessions[index].highlighter->setLanguage(Highlighter::languageFromName(selected));
}

void MainWindow::selectEncoding() {
    const int index = activeSessionIndex();
    if (index < 0) return;
    EditorSession &session = m_sessions[index];
    const QStringList encodings = { QStringLiteral("UTF-8"), QStringLiteral("UTF-16LE"),
                                    QStringLiteral("UTF-16BE"), QStringLiteral("ISO-8859-1"),
                                    QStringLiteral("System") };

    bool ok = false;
    const QString selected = QInputDialog::getItem(this,
        Lang::s(QStringLiteral("encoding")),
        Lang::s(QStringLiteral("encoding")),
        encodings, encodings.indexOf(session.encoding), false, &ok);
    if (!ok || selected.isEmpty()) return;

    session.encoding = selected;
    if (!session.path.isEmpty()) {
        QFile file(session.path);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray bytes = file.readAll();
            file.close();
            QString text = decodeBytes(bytes, session.encoding);
            session.crlf = text.contains(QStringLiteral("\r\n"));
            if (session.crlf) text.remove(QStringLiteral("\r"));
            session.editor->setPlainText(text);
            session.editor->document()->setModified(false);
        }
    }
}

void MainWindow::selectEol() {
    const int index = activeSessionIndex();
    if (index < 0) return;
    EditorSession &session = m_sessions[index];
    const QStringList modes = { QStringLiteral("LF"), QStringLiteral("CRLF") };

    bool ok = false;
    const QString selected = QInputDialog::getItem(this,
        Lang::s(QStringLiteral("new line mode")),
        Lang::s(QStringLiteral("new line mode")),
        modes, session.crlf ? 1 : 0, false, &ok);
    if (!ok || selected.isEmpty()) return;

    session.crlf = selected == QLatin1String("CRLF");
    showToast(Lang::s(QStringLiteral("new line mode")) + QStringLiteral(": ") + selected);
}

void MainWindow::formatActiveFile() {
    const int index = activeSessionIndex();
    if (index < 0) return;
    EditorSession &session = m_sessions[index];

    if (session.title.endsWith(QLatin1String(".json"), Qt::CaseInsensitive)) {
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(session.editor->toPlainText().toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            showToast(QStringLiteral("%1: %2").arg(Lang::s(QStringLiteral("error"))).arg(error.errorString()));
            return;
        }
        session.editor->setPlainText(QString::fromUtf8(document.toJson(QJsonDocument::Indented)));
    } else {
        showToast(QStringLiteral("Prettier is only available in the web app"));
    }
}

void MainWindow::insertColor() {
    if (!activeEditor()) return;
    const QColor color = QColorDialog::getColor(Theme::Active, this, Lang::s(QStringLiteral("insert color")));
    if (!color.isValid()) return;
    activeEditor()->insertAtCursor(color.name());
}

/* ------------------------------------------------------------------ */
/* Dialogs                                                             */
/* ------------------------------------------------------------------ */

void MainWindow::openFolderDialog() {
    const QString path = QFileDialog::getExistingDirectory(this, Lang::s(QStringLiteral("open folder")));
    if (path.isEmpty()) return;
    m_sidebar->addFolder(path);
    saveState();
}

void MainWindow::openFileDialog() {
    const QStringList paths = QFileDialog::getOpenFileNames(this, Lang::s(QStringLiteral("open file")));
    for (const QString &path : paths) openFile(path);
    saveState();
}

void MainWindow::findFileDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(Lang::s(QStringLiteral("find file")));
    dialog.resize(420, 480);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLineEdit *input = new QLineEdit(&dialog);
    input->setPlaceholderText(Lang::s(QStringLiteral("type filename")));
    input->setClearButtonEnabled(true);
    QListWidget *list = new QListWidget(&dialog);
    layout->addWidget(input);
    layout->addWidget(list, 1);

    QStringList allFiles;
    for (const QString &root : m_sidebar->folders()) {
        QDirIterator it(root, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext() && allFiles.size() < 2000) allFiles << it.next();
    }

    auto update = [list, &allFiles](const QString &term) {
        list->clear();
        int count = 0;
        for (const QString &path : allFiles) {
            if (term.isEmpty() || QFileInfo(path).fileName().contains(term, Qt::CaseInsensitive)) {
                QListWidgetItem *item = new QListWidgetItem(list);
                item->setText(QFileInfo(path).fileName());
                item->setData(Qt::UserRole, path);
                if (++count >= 100) break;
            }
        }
    };
    update(QString());
    connect(input, &QLineEdit::textChanged, &dialog, update);

    connect(list, &QListWidget::itemActivated, &dialog, [&](QListWidgetItem *item) {
        openFile(item->data(Qt::UserRole).toString());
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::recentDialog() {
    const QStringList recent = AppState::instance()->recentFiles();
    if (recent.isEmpty()) return;
    bool ok = false;
    const QString selected = QInputDialog::getItem(this,
        Lang::s(QStringLiteral("open recent")),
        Lang::s(QStringLiteral("recent")),
        recent, 0, false, &ok);
    if (!ok || selected.isEmpty()) return;
    openFile(selected);
}

void MainWindow::gotoLineDialog() {
    if (!activeEditor()) return;
    bool ok = false;
    const int line = QInputDialog::getInt(this,
        Lang::s(QStringLiteral("goto")),
        Lang::s(QStringLiteral("enter line number")),
        activeEditor()->currentLine(), 1, activeEditor()->blockCount(), 1, &ok);
    if (!ok) return;
    activeEditor()->gotoLine(line);
}

void MainWindow::consoleDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(Lang::s(QStringLiteral("console")));
    dialog.resize(480, 360);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QPlainTextEdit *logView = new QPlainTextEdit(&dialog);
    logView->setReadOnly(true);
    logView->setPlainText(m_consoleLog.join(QLatin1Char('\n')));
    layout->addWidget(logView);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    buttons->button(QDialogButtonBox::Close)->setText(Lang::s(QStringLiteral("close")));
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::clicked, &dialog, &QDialog::accept);
    layout->addWidget(buttons);
    dialog.exec();
}

void MainWindow::runningProcessesDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(Lang::s(QStringLiteral("running processes")));
    dialog.resize(420, 300);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *label = new QLabel(QStringLiteral("No running processes."), &dialog);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral("color: rgba(228,228,228,0.6);"));
    layout->addWidget(label);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    buttons->button(QDialogButtonBox::Close)->setText(Lang::s(QStringLiteral("close")));
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::clicked, &dialog, &QDialog::accept);
    layout->addWidget(buttons);
    dialog.exec();
}

void MainWindow::settingsDialog() {
    SettingsDialog dialog(this);
    connect(&dialog, &SettingsDialog::languageChanged, this, [this] {
        retranslate();
        showToast(Lang::s(QStringLiteral("language changed")));
    });
    dialog.exec();
}

void MainWindow::aboutDialog() {
    QMessageBox box(this);
    box.setWindowTitle(Lang::s(QStringLiteral("about")));
    box.setText(QStringLiteral("<b>CIDE</b><br>1.13.2<br><br>CIDE - Android code editor<br>MIT License"));
    box.setIconPixmap(QPixmap(QStringLiteral(":/res/logo.png")).scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    box.exec();
}

void MainWindow::helpPage() {
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://acode.app")));
}

/* ------------------------------------------------------------------ */
/* Search                                                              */
/* ------------------------------------------------------------------ */

void MainWindow::searchNext(const QString &term, bool matchCase, bool backwards) {
    CodeEditor *editor = activeEditor();
    if (!editor || term.isEmpty()) return;
    m_lastSearchTerm = term;
    m_lastMatchCase = matchCase;

    QTextDocument::FindFlags flags;
    if (matchCase) flags |= QTextDocument::FindCaseSensitively;
    if (backwards) flags |= QTextDocument::FindBackward;

    /* QTextEdit::find continues from the current selection */
    const bool found = editor->find(term, flags);
    if (!found) {
        /* wrap around */
        const QTextCursor restore = editor->textCursor();
        QTextCursor wrap = editor->textCursor();
        wrap.movePosition(backwards ? QTextCursor::End : QTextCursor::Start);
        editor->setTextCursor(wrap);
        if (!editor->find(term, flags)) {
            editor->setTextCursor(restore);
            updateSearchStatus(0, 0);
            return;
        }
    }

    /* compute position and total */
    int total = 0;
    int position = 0;
    QTextDocument *document = editor->document();
    QTextCursor scan(document);
    forever {
        scan = document->find(term, scan, matchCase ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags());
        if (scan.isNull()) break;
        ++total;
        if (scan.selectionStart() <= editor->textCursor().position() && scan.selectionEnd() >= editor->textCursor().position()) {
            position = total;
        }
    }
    updateSearchStatus(position > 0 ? position : 1, total);
}

void MainWindow::replaceNext(const QString &term, const QString &replacement, bool matchCase) {
    CodeEditor *editor = activeEditor();
    if (!editor || term.isEmpty()) return;

    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == term) {
        cursor.insertText(replacement);
    }
    searchNext(term, matchCase, false);
}

void MainWindow::replaceAll(const QString &term, const QString &replacement, bool matchCase) {
    CodeEditor *editor = activeEditor();
    if (!editor || term.isEmpty()) return;

    QTextDocument::FindFlags flags = matchCase ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags();
    int count = 0;
    QTextCursor cursor(editor->document());
    cursor.beginEditBlock();
    forever {
        cursor = editor->document()->find(term, cursor, flags);
        if (cursor.isNull()) break;
        cursor.insertText(replacement);
        ++count;
    }
    cursor.endEditBlock();
    updateSearchStatus(0, 0);
    showToast(QString::number(count));
}

void MainWindow::updateSearchStatus(int position, int total) {
    m_quickTools->updateSearchStatus(position, total);
}

/* ------------------------------------------------------------------ */
/* Session state                                                       */
/* ------------------------------------------------------------------ */

void MainWindow::saveState() {
    AppState *state = AppState::instance();
    state->setFolders(m_sidebar->folders());

    QStringList files;
    QStringList pinned;
    for (const EditorSession &session : m_sessions) {
        if (!session.path.isEmpty()) {
            files << session.path;
            if (session.pinned) pinned << session.path;
        }
    }
    state->setOpenFiles(state->rememberFiles() ? files : QStringList());
    state->setPinnedFiles(pinned);
    if (m_activeSession >= 0 && m_activeSession < m_sessions.size()) {
        state->setLastFile(m_sessions[m_activeSession].path);
    }
}

void MainWindow::restoreState() {
    AppState *state = AppState::instance();
    if (state->rememberFolders()) {
        m_sidebar->addFolders(state->folders());
    }
    if (state->rememberFiles()) {
        const QStringList files = state->openFiles();
        for (const QString &file : files) openFile(file);
        const QStringList pinned = state->pinnedFiles();
        for (EditorSession &session : m_sessions) {
            session.pinned = pinned.contains(session.path);
        }
        const QString last = state->lastFile();
        if (!last.isEmpty()) {
            for (int i = 0; i < m_sessions.size(); ++i) {
                if (m_sessions[i].path == last) {
                    setActiveTab(sessionToTabIndex(i));
                    break;
                }
            }
        }
    }
    if (state->quickToolsSearchMode()) m_quickTools->setSearchMode(true);
    rebuildTabs();
}

/* ------------------------------------------------------------------ */
/* Misc                                                                */
/* ------------------------------------------------------------------ */

void MainWindow::showToast(const QString &message) {
    m_toast->setText(message);
    m_toast->adjustSize();
    const int y = m_central->height() - m_quickTools->height() - m_toast->height() - 24;
    m_toast->move((m_central->width() - m_toast->width()) / 2, y);
    m_toast->show();
    m_toast->raise();
    QTimer::singleShot(2500, m_toast, [this] { m_toast->hide(); });
}

void MainWindow::log(const QString &message) {
    m_consoleLog.append(message);
}

void MainWindow::applySettingsToEditors() {
    for (EditorSession &session : m_sessions) {
        if (session.editor) session.editor->applySettings();
    }
}

void MainWindow::retranslate() {
    m_sidebar->retranslate();
    m_quickTools->retranslate();
    m_welcome->retranslate();
    updateHeader();
    rebuildTabs();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    const int sidebarWidth = qBound(180, int(width() * 0.7), 350);
    m_sidebar->setFixedWidth(sidebarWidth);
    if (width() >= 720) m_sidebar->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    for (const EditorSession &session : m_sessions) {
        if (session.editor->document()->isModified()) {
            const QMessageBox::StandardButton answer = QMessageBox::question(
                this,
                QStringLiteral("CIDE"),
                Lang::s(QStringLiteral("unsaved files close app")),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
            if (answer != QMessageBox::Yes) {
                event->ignore();
                return;
            }
            break;
        }
    }
    saveState();
    event->accept();
}
