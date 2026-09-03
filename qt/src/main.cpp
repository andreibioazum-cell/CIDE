#include <QApplication>

#include "appstate.h"
#include "lang.h"
#include "mainwindow.h"
#include "theme.h"

int main(int argc, char *argv[]) {
    /* The interface of CIDE (commit 3673f5a) reimplemented with
     * pure C++ / Qt Widgets — no QML. */
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CIDE"));
    QApplication::setOrganizationName(QStringLiteral("CIDE"));
    QApplication::setApplicationDisplayName(QStringLiteral("CIDE"));
    QApplication::setApplicationVersion(QStringLiteral("1.13.2"));

    Lang::setFromCode(AppState::instance()->langCode());
    Theme::apply();

    MainWindow window;
    if (AppState::instance()->fullscreen()) {
        window.showFullScreen();
    } else {
        window.show();
    }

    return app.exec();
}
