#include "filetree.h"
#include "headerbar.h"
#include "icons.h"
#include "lang.h"
#include "theme.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {

constexpr int kRolePath = Qt::UserRole + 1;
constexpr int kRoleIsDir = Qt::UserRole + 2;
constexpr int kRoleLoaded = Qt::UserRole + 3;

bool entryLessThan(const QFileInfo &a, const QFileInfo &b) {
    if (a.isDir() != b.isDir()) return a.isDir();
    return a.fileName().toLower() < b.fileName().toLower();
}

} // namespace

/* ------------------------------------------------------------------ */
/* FileTree                                                            */
/* ------------------------------------------------------------------ */

FileTree::FileTree(QWidget *parent) : QWidget(parent) {
    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setFrameShape(QFrame::NoFrame);
    m_tree->setUniformRowHeights(true);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree->header()->setStretchLastSection(false);
    m_tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(m_tree, &QTreeWidget::itemActivated, this, &FileTree::onItemActivated);
    connect(m_tree, &QTreeWidget::itemClicked, this, &FileTree::onItemActivated);
    connect(m_tree, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem *item) {
        populate(item);
    });
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &FileTree::onContextMenu);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_tree);
}

QStringList FileTree::folders() const {
    QStringList result;
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        result << m_tree->topLevelItem(i)->data(0, kRolePath).toString();
    }
    return result;
}

bool FileTree::hasFolder(const QString &path) const {
    return folders().contains(QDir::cleanPath(path));
}

void FileTree::addFolder(const QString &path) {
    const QString clean = QDir::cleanPath(path);
    if (hasFolder(clean)) return;

    QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
    item->setText(0, QDir(clean).dirName().isEmpty() ? clean : QDir(clean).dirName());
    item->setData(0, kRolePath, clean);
    item->setData(0, kRoleIsDir, true);
    item->setData(0, kRoleLoaded, false);
    item->setIcon(0, Icons::fileIcon(QString(), true));
    item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    m_tree->addTopLevelItem(item);
    m_tree->expandItem(item);
    emit treeChanged();
}

void FileTree::refreshFolder(const QString &path) {
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_tree->topLevelItem(i);
        if (item->data(0, kRolePath).toString() == QDir::cleanPath(path)) {
            item->takeChildren();
            item->setData(0, kRoleLoaded, false);
            populate(item);
        }
    }
}

void FileTree::populate(QTreeWidgetItem *item) {
    if (item->data(0, kRoleLoaded).toBool()) return;
    item->setData(0, kRoleLoaded, true);

    const QString dirPath = item->data(0, kRolePath).toString();
    const QFileInfoList entries = QDir(dirPath).entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden,
        QDir::DirsFirst | QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);

    for (const QFileInfo &entry : entries) {
        QTreeWidgetItem *child = new QTreeWidgetItem(item);
        child->setText(0, entry.fileName());
        child->setData(0, kRolePath, entry.absoluteFilePath());
        child->setData(0, kRoleIsDir, entry.isDir());
        child->setData(0, kRoleLoaded, false);
        child->setIcon(0, Icons::fileIcon(entry.fileName(), entry.isDir()));
        if (entry.isDir()) {
            child->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
        item->addChild(child);
    }
    m_tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void FileTree::onItemActivated(QTreeWidgetItem *item, int) {
    if (!item || item->data(0, kRoleIsDir).toBool()) return;
    const QString path = item->data(0, kRolePath).toString();
    if (!path.isEmpty()) emit fileActivated(path);
}

void FileTree::onContextMenu(const QPoint &pos) {
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    QString targetDir;
    if (item) {
        const bool isDir = item->data(0, kRoleIsDir).toBool();
        if (isDir) {
            targetDir = item->data(0, kRolePath).toString();
        } else {
            targetDir = QFileInfo(item->data(0, kRolePath).toString()).absolutePath();
        }
    }

    QMenu menu(this);
    if (item) {
        const bool isDir = item->data(0, kRoleIsDir).toBool();
        if (isDir) {
            QAction *newFileAction = menu.addAction(Icons::icon(Icons::Add), Lang::s(QStringLiteral("new file")));
            connect(newFileAction, &QAction::triggered, this, [this, targetDir] { newFile(targetDir); });
            QAction *newFolderAction = menu.addAction(Icons::icon(Icons::Folder), Lang::s(QStringLiteral("new folder")));
            connect(newFolderAction, &QAction::triggered, this, [this, targetDir] { newFolder(targetDir); });
            menu.addSeparator();
        }
        QAction *renameAction = menu.addAction(Lang::s(QStringLiteral("rename")));
        connect(renameAction, &QAction::triggered, this, [this, item] { renameEntry(item); });
        QAction *deleteAction = menu.addAction(Lang::s(QStringLiteral("delete")));
        connect(deleteAction, &QAction::triggered, this, [this, item] { deleteEntry(item); });
    } else {
        QAction *openAction = menu.addAction(Icons::icon(Icons::FolderOpen), Lang::s(QStringLiteral("open folder")));
        connect(openAction, &QAction::triggered, this, [this] { emit folderAdded(QString()); });
        menu.addSeparator();
        QAction *newFileAction = menu.addAction(Icons::icon(Icons::Add), Lang::s(QStringLiteral("new file")));
        connect(newFileAction, &QAction::triggered, this, [this, targetDir] { newFile(targetDir); });
    }
    menu.exec(m_tree->viewport()->mapToGlobal(pos));
}

void FileTree::newFile(const QString &dir) {
    QString targetDir = dir;
    if (targetDir.isEmpty()) {
        const QStringList roots = folders();
        if (roots.isEmpty()) {
            emit folderAdded(QString());
            return;
        }
        targetDir = roots.first();
    }

    QInputDialog dialog(this);
    dialog.setWindowTitle(Lang::s(QStringLiteral("new file")));
    dialog.setLabelText(Lang::s(QStringLiteral("type filename")));
    dialog.setInputMode(QInputDialog::TextInput);
    int code = dialog.exec();
    if (code != QDialog::Accepted) return;
    const QString name = dialog.textValue().trimmed();
    if (name.isEmpty()) return;

    const QString path = QDir(targetDir).filePath(name);
    if (QFile::exists(path)) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("file already exists")));
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("unable to save file")));
        return;
    }
    file.close();
    refreshFolder(targetDir);
    emit treeChanged();
    emit fileActivated(path);
}

void FileTree::newFolder(const QString &dir) {
    if (dir.isEmpty()) return;
    QInputDialog dialog(this);
    dialog.setWindowTitle(Lang::s(QStringLiteral("new folder")));
    dialog.setLabelText(Lang::s(QStringLiteral("folder name")));
    dialog.setInputMode(QInputDialog::TextInput);
    if (dialog.exec() != QDialog::Accepted) return;
    const QString name = dialog.textValue().trimmed();
    if (name.isEmpty()) return;

    if (!QDir(dir).mkdir(name)) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("create folder error")));
        return;
    }
    refreshFolder(dir);
    emit treeChanged();
}

void FileTree::renameEntry(QTreeWidgetItem *item) {
    const QString path = item->data(0, kRolePath).toString();
    const QFileInfo info(path);
    const QString parentDir = info.absolutePath();

    QInputDialog dialog(this);
    dialog.setWindowTitle(Lang::s(QStringLiteral("rename")));
    dialog.setLabelText(Lang::s(QStringLiteral("enter new name")));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextValue(info.fileName());
    if (dialog.exec() != QDialog::Accepted) return;
    const QString newName = dialog.textValue().trimmed();
    if (newName.isEmpty() || newName == info.fileName()) return;

    const QString target = QDir(parentDir).filePath(newName);
    QFile file(path);
    if (!file.rename(target)) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("unable to rename")));
        return;
    }
    refreshFolder(parentDir);
    emit treeChanged();
}

void FileTree::deleteEntry(QTreeWidgetItem *item) {
    const QString path = item->data(0, kRolePath).toString();
    const QFileInfo info(path);

    const QString question = Lang::s(QStringLiteral("delete entry")).replace(
        QStringLiteral("{name}"), info.fileName());
    if (QMessageBox::question(this, Lang::s(QStringLiteral("delete")), question) != QMessageBox::Yes) {
        return;
    }

    bool ok = false;
    if (info.isDir()) {
        ok = QDir(path).removeRecursively();
    } else {
        ok = QFile::remove(path);
    }
    if (!ok) {
        QMessageBox::warning(this, Lang::s(QStringLiteral("an error occurred")),
                             Lang::s(QStringLiteral("unable to delete file")));
        return;
    }
    refreshFolder(info.absolutePath());
    emit treeChanged();
}

/* ------------------------------------------------------------------ */
/* Sidebar::RailButton                                                 */
/* ------------------------------------------------------------------ */

class Sidebar::RailButton : public QWidget {
    Q_OBJECT
public:
    RailButton(Icons::Icon iconKind, const QString &tooltip, QWidget *parent = nullptr)
        : QWidget(parent), m_iconKind(iconKind) {
        setFixedSize(40, 40);
        setToolTip(tooltip);
        setCursor(Qt::PointingHandCursor);
        setMouseTracking(true);
    }

    void setActive(bool active) {
        m_active = active;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        if (m_active) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(Theme::railActive());
            painter.drawRoundedRect(rect().adjusted(3, 3, -3, -3), 10, 10);
            /* left indicator bar */
            painter.setBrush(Theme::PrimaryText);
            painter.drawRoundedRect(QRectF(1, 12, 3, 16), 0, 2);
        } else if (m_hover) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(Theme::railHover());
            painter.drawRoundedRect(rect().adjusted(3, 3, -3, -3), 10, 10);
        }

        painter.setOpacity(m_active ? 1.0 : 0.55);
        const QIcon icon = Icons::icon(m_iconKind, Theme::PrimaryText);
        painter.drawPixmap(8, 8, icon.pixmap(QSize(24, 24)));
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            m_pressed = true;
            update();
        }
        QWidget::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && m_pressed && rect().contains(event->pos())) {
            m_pressed = false;
            update();
            emit activated();
        }
        QWidget::mouseReleaseEvent(event);
    }

    void enterEvent(QEnterEvent *event) override {
        m_hover = true;
        update();
        QWidget::enterEvent(event);
    }

    void leaveEvent(QEvent *event) override {
        m_hover = false;
        m_pressed = false;
        update();
        QWidget::leaveEvent(event);
    }

signals:
    void activated();

private:
    Icons::Icon m_iconKind;
    bool m_active = false;
    bool m_hover = false;
    bool m_pressed = false;
};

/* ------------------------------------------------------------------ */
/* Sidebar                                                             */
/* ------------------------------------------------------------------ */

Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("sidebarContainer"));
    buildRail();
    buildPanels();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_rail);
    layout->addWidget(m_panels, 1);

    setMinimumWidth(180); /* #sidebar { width: 70vw; max-width: 350px } — overlay drawer */
    setActiveApp(FilesApp);
}

void Sidebar::buildRail() {
    m_rail = new QWidget(this);
    m_rail->setFixedWidth(52); /* .apps width */
    m_rail->setStyleSheet(QStringLiteral("background-color: #1e2225;")); /* primary + rgba(0,0,0,.15) */

    m_railLayout = new QVBoxLayout(m_rail);
    m_railLayout->setContentsMargins(0, 8, 0, 8);
    m_railLayout->setSpacing(4);
    m_railLayout->setAlignment(Qt::AlignTop);
}

void Sidebar::buildPanels() {
    m_panels = new QStackedWidget(this);

    /* --- files panel --- */
    QWidget *filesPanel = new QWidget(m_panels);
    QVBoxLayout *filesLayout = new QVBoxLayout(filesPanel);
    filesLayout->setContentsMargins(0, 0, 0, 0);
    filesLayout->setSpacing(0);

    m_fileTree = new FileTree(filesPanel);
    connect(m_fileTree, &FileTree::fileActivated, this, &Sidebar::fileActivated);
    connect(m_fileTree, &FileTree::folderAdded, this, [this](const QString &) {
        emit openFolderRequested();
    });

    m_emptyOpenFolder = new QPushButton(Lang::s(QStringLiteral("open folder")), filesPanel);
    connect(m_emptyOpenFolder, &QPushButton::clicked, this, [this] { emit openFolderRequested(); });

    QLabel *emptyLabel = new QLabel(Lang::s(QStringLiteral("open folder")), filesPanel);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet(QStringLiteral("color: rgba(245,245,245,0.6); font-size: 13px; padding: 24px;"));

    QVBoxLayout *emptyLayout = new QVBoxLayout;
    emptyLayout->addStretch(1);
    emptyLayout->addWidget(emptyLabel, 0, Qt::AlignHCenter);
    emptyLayout->addWidget(m_emptyOpenFolder, 0, Qt::AlignHCenter);
    emptyLayout->addStretch(1);

    QWidget *emptyWidget = new QWidget(filesPanel);
    emptyWidget->setLayout(emptyLayout);

    QStackedWidget *filesStack = new QStackedWidget(filesPanel);
    filesStack->addWidget(m_fileTree);
    filesStack->addWidget(emptyWidget);
    filesStack->setCurrentIndex(1);

    connect(m_fileTree, &FileTree::treeChanged, this, [filesStack, this] {
        filesStack->setCurrentIndex(m_fileTree->folders().isEmpty() ? 1 : 0);
    });

    filesLayout->addWidget(filesStack);
    m_panels->addWidget(filesPanel);

    /* --- search panel --- */
    QWidget *searchPanel = new QWidget(m_panels);
    QVBoxLayout *searchLayout = new QVBoxLayout(searchPanel);
    searchLayout->setContentsMargins(8, 8, 8, 8);
    searchLayout->setSpacing(8);

    m_searchInput = new QLineEdit(searchPanel);
    m_searchInput->setPlaceholderText(Lang::s(QStringLiteral("search in files")));
    m_searchInput->setClearButtonEnabled(true);

    m_replaceInput = new QLineEdit(searchPanel);
    m_replaceInput->setPlaceholderText(Lang::s(QStringLiteral("replace")));

    QPushButton *searchButton = new QPushButton(Lang::s(QStringLiteral("search")), searchPanel);
    connect(searchButton, &QPushButton::clicked, this, &Sidebar::runSearch);
    connect(m_searchInput, &QLineEdit::returnPressed, this, &Sidebar::runSearch);

    m_searchStatus = new QLabel(searchPanel);
    m_searchStatus->setStyleSheet(QStringLiteral("color: rgba(245,245,245,0.6); font-size: 12px;"));

    m_searchResults = new QListWidget(searchPanel);
    connect(m_searchResults, &QListWidget::itemActivated, this, [this](QListWidgetItem *item) {
        const int line = item->data(Qt::UserRole + 1).toInt();
        const QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) emit searchResultActivated(path, line);
    });
    connect(m_searchResults, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const int line = item->data(Qt::UserRole + 1).toInt();
        const QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) emit searchResultActivated(path, line);
    });

    searchLayout->addWidget(m_searchInput);
    searchLayout->addWidget(m_replaceInput);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(m_searchStatus);
    searchLayout->addWidget(m_searchResults, 1);
    m_panels->addWidget(searchPanel);

    /* --- extensions panel --- */
    QWidget *extensionsPanel = new QWidget(m_panels);
    QVBoxLayout *extLayout = new QVBoxLayout(extensionsPanel);
    extLayout->setContentsMargins(16, 16, 16, 16);
    m_extensionsLabel = new QLabel(extensionsPanel);
    m_extensionsLabel->setWordWrap(true);
    m_extensionsLabel->setAlignment(Qt::AlignCenter);
    m_extensionsLabel->setStyleSheet(QStringLiteral("color: rgba(245,245,245,0.6); font-size: 13px;"));
    extLayout->addStretch(1);
    extLayout->addWidget(m_extensionsLabel);
    extLayout->addStretch(1);
    m_panels->addWidget(extensionsPanel);

    /* --- notifications panel --- */
    QWidget *notificationsPanel = new QWidget(m_panels);
    QVBoxLayout *notifLayout = new QVBoxLayout(notificationsPanel);
    notifLayout->setContentsMargins(16, 16, 16, 16);
    m_notificationsLabel = new QLabel(notificationsPanel);
    m_notificationsLabel->setWordWrap(true);
    m_notificationsLabel->setAlignment(Qt::AlignCenter);
    m_notificationsLabel->setStyleSheet(QStringLiteral("color: rgba(245,245,245,0.6); font-size: 13px;"));
    notifLayout->addStretch(1);
    notifLayout->addWidget(m_notificationsLabel);
    notifLayout->addStretch(1);
    m_panels->addWidget(notificationsPanel);

    retranslate();
}

void Sidebar::retranslate() {
    /* rebuild rail */
    while (!m_railButtons.isEmpty()) {
        RailButton *button = m_railButtons.takeFirst();
        m_railLayout->removeWidget(button);
        button->deleteLater();
    }

    struct AppDef {
        Icons::Icon icon;
        const char *tooltipKey;
        App app;
    };
    const AppDef defs[] = {
        { Icons::Documents, "files", FilesApp },
        { Icons::Search, "search in files", SearchApp },
        { Icons::Extension, "plugins", ExtensionsApp },
        { Icons::Notifications, "notifications", NotificationsApp },
    };
    for (const AppDef &def : defs) {
        RailButton *button = new RailButton(def.icon, Lang::s(QString::fromLatin1(def.tooltipKey)), m_rail);
        button->setActive(def.app == FilesApp);
        connect(button, &RailButton::activated, this, [this, def, button] {
            setActiveApp(def.app);
            for (RailButton *other : m_railButtons) other->setActive(other == button);
        });
        m_railLayout->addWidget(button, 0, Qt::AlignHCenter);
        m_railButtons.append(button);
    }

    m_emptyOpenFolder->setText(Lang::s(QStringLiteral("open folder")));
    m_searchInput->setPlaceholderText(Lang::s(QStringLiteral("search in files")));
    m_replaceInput->setPlaceholderText(Lang::s(QStringLiteral("replace")));
    m_extensionsLabel->setText(Lang::s(QStringLiteral("no plugins found")));
    m_notificationsLabel->setText(Lang::s(QStringLiteral("no_unread_notifications")));
}

void Sidebar::setActiveApp(App app) {
    m_panels->setCurrentIndex(int(app));
    for (int i = 0; i < m_railButtons.size(); ++i) {
        m_railButtons[i]->setActive(i == int(app));
    }
}

void Sidebar::addFolder(const QString &path) {
    m_fileTree->addFolder(path);
}

void Sidebar::addFolders(const QStringList &paths) {
    for (const QString &path : paths) m_fileTree->addFolder(path);
}

QStringList Sidebar::folders() const {
    return m_fileTree->folders();
}

void Sidebar::runSearch() {
    const QString term = m_searchInput->text();
    m_searchResults->clear();
    if (term.isEmpty()) {
        m_searchStatus->setText(QString());
        return;
    }

    int matches = 0;
    int filesSearched = 0;
    for (const QString &root : m_fileTree->folders()) {
        QDirIterator it(root, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext() && matches < 200) {
            const QString path = it.next();
            const QFileInfo info(path);
            if (info.size() > 2 * 1024 * 1024) continue;
            ++filesSearched;
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly)) continue;
            int line = 0;
            int fileMatches = 0;
            while (!file.atEnd() && fileMatches < 20 && matches < 200) {
                const QString text = QString::fromUtf8(file.readLine());
                ++line;
                if (text.contains(term, Qt::CaseInsensitive)) {
                    ++matches;
                    ++fileMatches;
                    QListWidgetItem *item = new QListWidgetItem(m_searchResults);
                    item->setText(QStringLiteral("%1\n%2: %3").arg(
                        info.fileName(),
                        QString::number(line),
                        text.left(120).simplified()));
                    item->setData(Qt::UserRole, path);
                    item->setData(Qt::UserRole + 1, line);
                }
            }
        }
    }
    m_searchStatus->setText(QStringLiteral("%1 / %2").arg(matches).arg(filesSearched));
}

#include "filetree.moc"
