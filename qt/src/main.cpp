#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWebView/QtWebView>
#include "editorcontroller.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QtWebView::initialize();
    qmlRegisterType<EditorController>("CIDE", 1, 0, "EditorController");
    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/CIDE/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
