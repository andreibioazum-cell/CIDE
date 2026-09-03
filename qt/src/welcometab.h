#pragma once

#include <QScrollArea>
#include <QUrl>

#include "icons.h"

#include <functional>

class QLabel;

/*
 * The "Welcome to CIDE" tab (src/pages/welcome at commit 3673f5a):
 * hero header, GET STARTED / CONFIGURE / LEARN / CONNECT sections.
 */
class WelcomeTab : public QScrollArea {
    Q_OBJECT

public:
    explicit WelcomeTab(QWidget *parent = nullptr);
    void retranslate();

signals:
    void newFileRequested();
    void openFileRequested();
    void openFolderRequested();
    void openTerminalRequested();
    void openRecentRequested();
    void commandPaletteRequested();
    void openSettingsRequested();
    void explorePluginsRequested();
    void helpRequested();
    void aboutRequested();

private:
    QWidget *makeActionRow(Icons::Icon icon, const QString &label, const std::function<void()> &handler);
    QWidget *makeLinkRow(const QString &label, const QUrl &url);

    QLabel *m_titleLabel = nullptr;
    QLabel *m_taglineLabel = nullptr;
    QWidget *m_content = nullptr;
    QList<QLabel *> m_sectionLabels;
};
