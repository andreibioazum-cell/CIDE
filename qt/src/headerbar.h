#pragma once

#include <QWidget>

class QHBoxLayout;
class QLabel;
class QVBoxLayout;

/*
 * Flat icon button used in the header and quick tools.
 * Painted like the web ".icon" elements (45x45 in header, transparent
 * background, rgba(0,0,0,0.2) feedback on press/active).
 */
class IconButton : public QWidget {
    Q_OBJECT

public:
    explicit IconButton(QWidget *parent = nullptr);

    void setIcon(const QIcon &icon, int extent = 24);
    void setText(const QString &text, int pixelSize = 12);
    void setActive(bool active);
    void setBadge(bool badge);
    bool isActive() const { return m_active; }
    void setFontPixelSize(int size);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sizeHint() const override;

private:
    QIcon m_icon;
    QString m_text;
    int m_iconExtent;
    int m_textSize;
    bool m_active = false;
    bool m_hover = false;
    bool m_pressed = false;
    bool m_badge = false;
};

/*
 * App header (components/tile type=header): 45px tall, primary color,
 * bold 1.2em title with 0.58em sub-text, nav toggler on the left and
 * the "more" toggler on the right. The edit (file menu) toggler and the
 * run button appear between title and more button for editor files.
 */
class HeaderBar : public QWidget {
    Q_OBJECT

public:
    explicit HeaderBar(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setSubTitle(const QString &subTitle);
    void setFileMenuVisible(bool visible);
    void setRunVisible(bool visible);

signals:
    void navToggled();
    void menuToggled();
    void fileMenuToggled();
    void runClicked();

private:
    QHBoxLayout *m_layout = nullptr;
    IconButton *m_navButton = nullptr;
    IconButton *m_editButton = nullptr;
    IconButton *m_runButton = nullptr;
    IconButton *m_moreButton = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subLabel = nullptr;
};
