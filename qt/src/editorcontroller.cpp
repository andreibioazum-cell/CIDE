#include "editorcontroller.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

QString EditorController::openFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    m_file = QFileInfo(file).absoluteFilePath();
    emit currentFileChanged();
    return QString::fromUtf8(file.readAll());
}

bool EditorController::saveFile(const QString &path, const QString &text) {
    const QString target = path.isEmpty() ? m_file : path;
    if (target.isEmpty()) return false;
    QFile file(target);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    file.write(text.toUtf8());
    m_file = QFileInfo(file).absoluteFilePath();
    emit currentFileChanged();
    return true;
}

QString EditorController::homePath() const { return QDir::homePath(); }
