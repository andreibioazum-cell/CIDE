#pragma once

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;

class IconButton;

/*
 * File tree of an opened folder ("storage"), lazy-loaded, folders first.
 * Mirrors components/fileTree + sidebarApps/files at commit 3673f5a.
 */
class FileTree : public QWidget {
    Q_OBJECT

public:
    explicit FileTree(QWidget *parent = nullptr);

    void addFolder(const QString &path);
    QStringList folders() const;
    bool hasFolder(const QString &path) const;
    void refreshFolder(const QString &path);

signals:
    void fileActivated(const QString &path);
    void folderAdded(const QString &path);
    void treeChanged();

private:
    void populate(QTreeWidgetItem *item);
    void onItemActivated(QTreeWidgetItem *item, int column);
    void onContextMenu(const QPoint &pos);
    void newFile(const QString &dir);
    void newFolder(const QString &dir);
    void renameEntry(QTreeWidgetItem *item);
    void deleteEntry(QTreeWidgetItem *item);

    QTreeWidget *m_tree = nullptr;
};

/*
 * Sidebar: 52px icon rail (apps) + container with panels.
 * Apps: Files, Search, Extensions, Notifications (sidebarApps/index.js).
 */
class Sidebar : public QWidget {
    Q_OBJECT

public:
    enum App { FilesApp = 0, SearchApp, ExtensionsApp, NotificationsApp };

    explicit Sidebar(QWidget *parent = nullptr);

    void setActiveApp(App app);
    void addFolder(const QString &path);
    void addFolders(const QStringList &paths);
    QStringList folders() const;

    void retranslate();

signals:
    void fileActivated(const QString &path);
    void openFolderRequested();
    void searchResultActivated(const QString &path, int line);

private:
    class RailButton;
    void buildRail();
    void buildPanels();
    void runSearch();

    QWidget *m_rail = nullptr;
    QVBoxLayout *m_railLayout = nullptr;
    QList<RailButton *> m_railButtons;
    QStackedWidget *m_panels = nullptr;

    FileTree *m_fileTree = nullptr;
    QLineEdit *m_searchInput = nullptr;
    QLineEdit *m_replaceInput = nullptr;
    QListWidget *m_searchResults = nullptr;
    QLabel *m_searchStatus = nullptr;
    QLabel *m_extensionsLabel = nullptr;
    QLabel *m_notificationsLabel = nullptr;
    QPushButton *m_emptyOpenFolder = nullptr;
};
