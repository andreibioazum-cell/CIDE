#pragma once

#include <QWidget>

class QScrollArea;
class QHBoxLayout;

struct FileTabData {
    QString id;        /* file path or synthetic id */
    QString title;     /* tab title */
    QString fileName;  /* used for the file type icon */
    bool unsaved = false;
    bool pinned = false;
    bool closable = true;
    bool isWelcome = false;
};

/*
 * The "open-file-list" strip under the header: 30px tall, primary
 * background, 120px wide tabs with an unsaved "•" marker, active tab
 * with a 2px top border in the active color.
 */
class TabButton : public QWidget {
    Q_OBJECT

public:
    explicit TabButton(QWidget *parent = nullptr);

    void setData(const FileTabData &data);
    const FileTabData &data() const { return m_data; }
    void setActive(bool active);

signals:
    void clicked();
    void contextMenuRequested(const QPoint &globalPos);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    FileTabData m_data;
    bool m_active = false;
    bool m_hover = false;
};

class FileTabs : public QWidget {
    Q_OBJECT

public:
    explicit FileTabs(QWidget *parent = nullptr);

    void setTabs(const QList<FileTabData> &tabs);
    void setActiveIndex(int index);
    int activeIndex() const { return m_activeIndex; }
    int indexOf(const QString &id) const;
    void ensureVisible(int index);

signals:
    void tabClicked(int index);
    void tabContextMenu(int index, const QPoint &globalPos);

private:
    QScrollArea *m_scroll = nullptr;
    QWidget *m_container = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QList<TabButton *> m_buttons;
    int m_activeIndex = -1;
};
