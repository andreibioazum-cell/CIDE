#pragma once
#include <QObject>
#include <QString>

class EditorController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY currentFileChanged)
public:
    explicit EditorController(QObject *parent = nullptr) : QObject(parent) {}
    QString currentFile() const { return m_file; }
    Q_INVOKABLE QString openFile(const QString &path);
    Q_INVOKABLE bool saveFile(const QString &path, const QString &text);
    Q_INVOKABLE QString homePath() const;
signals:
    void currentFileChanged();
private:
    QString m_file;
};
