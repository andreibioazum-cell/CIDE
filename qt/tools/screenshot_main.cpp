#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTimer>

#include "appstate.h"
#include "lang.h"
#include "mainwindow.h"
#include "theme.h"

/*
 * Offscreen screenshot tool used for testing the interface without a
 * display: renders the main window and saves PNG snapshots.
 * Not built for Android.
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CIDE"));
    QApplication::setOrganizationName(QStringLiteral("CIDE"));

    const QString outputDir = QApplication::arguments().value(1, QDir::currentPath());
    const QString lang = QApplication::arguments().value(2, QStringLiteral("en-us"));
    Lang::setFromCode(lang);
    Theme::apply();

    /* sample project for the screenshots */
    const QDir sample(QDir::temp().absoluteFilePath(QStringLiteral("cide-sample")));
    sample.mkpath(QStringLiteral("."));
    const QString projectDir = sample.absolutePath();

    QFile mainFile(sample.absoluteFilePath(QStringLiteral("index.html")));
    mainFile.open(QIODevice::WriteOnly);
    mainFile.write("<!DOCTYPE html>\n<html>\n  <head>\n    <title>CIDE</title>\n    <link rel=\"stylesheet\" href=\"style.css\" />\n  </head>\n  <body>\n    <h1>Hello, CIDE!</h1>\n    <script src=\"app.js\"></script>\n  </body>\n</html>\n");
    mainFile.close();

    QFile cssFile(sample.absoluteFilePath(QStringLiteral("style.css")));
    cssFile.open(QIODevice::WriteOnly);
    cssFile.write("body {\n  margin: 0;\n  color: #abb2bf;\n  background-color: #282c34;\n}\n");
    cssFile.close();

    QFile jsFile(sample.absoluteFilePath(QStringLiteral("app.js")));
    jsFile.open(QIODevice::WriteOnly);
    jsFile.write("const greeting = 'Hello, world!';\n\nfunction greet(name) {\n  // say hello\n  if (!name) return;\n  console.log(`${greeting} ${name}`);\n}\n");
    jsFile.close();

    MainWindow window;
    window.sidebar()->addFolder(projectDir);
    window.openFileIn(sample.absoluteFilePath(QStringLiteral("app.js")));
    window.resize(420, 840);
    window.show();
    /* the web sidebar is a closed drawer by default — phone shots without it */
    window.repaint();
    QApplication::processEvents();

    QTimer::singleShot(600, &app, [&window, &outputDir, &lang, &app] {
        window.grab().save(QStringLiteral("%1/cide-%2-phone.png").arg(outputDir, lang));
        window.activateTab(0); /* welcome tab exercises the localized strings */
        window.resize(420, 1600); /* tall window: fit the whole welcome page */
        QApplication::processEvents();
        QTimer::singleShot(400, &app, [&window, &outputDir, &lang, &app] {
            window.grab().save(QStringLiteral("%1/cide-%2-welcome.png").arg(outputDir, lang));
            window.activateTab(1); /* back to the editor session (shows quick tools again) */
            window.resize(1000, 800);
            QApplication::processEvents();
            QTimer::singleShot(400, &app, [&window, &outputDir, &lang, &app] {
                window.grab().save(QStringLiteral("%1/cide-%2-wide.png").arg(outputDir, lang));
                window.showSidebar(); /* drawer open over the editor, like the web */
                QApplication::processEvents();
                QTimer::singleShot(400, &app, [&window, &outputDir, &lang] {
                    window.grab().save(QStringLiteral("%1/cide-%2-sidebar.png").arg(outputDir, lang));
                    QApplication::quit();
                });
            });
        });
    });

    return app.exec();
}
