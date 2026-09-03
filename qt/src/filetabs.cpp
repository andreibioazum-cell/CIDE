#include "filetabs.h"
#include "icons.h"
#include "theme.h"

#include <QContextMenuEvent>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEvent>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
constexpr int kTabWidth = 120; /* --file-tab-width */
constexpr int kTabHeight = 30; /* .open-file-list height */
constexpr int kStripHeight = 30;
} // namespace

/* ------------------------------------------------------------------ */
/* TabButton                                                           */
/* ------------------------------------------------------------------ */

TabButton::TabButton(QWidget *parent) : QWidget(parent) {
    setFixedSize(kTabWidth, kTabHeight);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
}

void TabButton::setData(const FileTabData &data) {
    m_data = data;
    update();
}

void TabButton::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;
    update();
}

void TabButton::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_active) {
        painter.fillRect(rect(), Theme::tabActiveBackground());
        painter.setPen(Qt::NoPen);
        painter.setBrush(Theme::Active);
        painter.drawRect(0, 0, width(), 2); /* border-top: 2px active */
    } else if (m_hover) {
        painter.fillRect(rect(), QColor(255, 255, 255, 14));
    }

    /* unsaved marker "•" (#ffda0c) */
    int x = 6;
    if (m_data.unsaved) {
        painter.setPen(Theme::Notice);
        QFont font = this->font();
        font.setBold(true);
        font.setPixelSize(13);
        painter.setFont(font);
        painter.drawText(QRect(x, 0, 8, height()), Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("•"));
        x += 8;
    }

    /* file icon */
    const QIcon icon = m_data.isWelcome
        ? QIcon(QStringLiteral(":/res/logo.png"))
        : Icons::fileIcon(m_data.fileName, false);
    const int iconExtent = 14;
    const QPixmap pixmap = icon.pixmap(QSize(iconExtent, iconExtent));
    if (!pixmap.isNull()) {
        painter.drawPixmap(x, (height() - iconExtent) / 2, pixmap);
    }
    x += iconExtent + 4;

    /* title with ellipsis */
    QFont font = this->font();
    font.setPixelSize(11); /* 0.8em */
    painter.setFont(font);
    painter.setPen(Theme::PrimaryText);
    const int textWidth = width() - x - (m_data.pinned ? 16 : 6);
    if (textWidth > 4) {
        const QString elided = fontMetrics().elidedText(m_data.title, Qt::ElideMiddle, textWidth);
        painter.drawText(QRect(x, 0, textWidth, height()), Qt::AlignVCenter | Qt::AlignLeft, elided);
    }

    /* pinned marker */
    if (m_data.pinned) {
        const QIcon pin = Icons::icon(Icons::Pin, QColor(245, 245, 245, 160));
        painter.drawPixmap(width() - 14, (height() - 10) / 2, pin.pixmap(QSize(10, 10)));
    }
}

void TabButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) emit clicked();
    QWidget::mousePressEvent(event);
}

void TabButton::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
}

void TabButton::contextMenuEvent(QContextMenuEvent *event) {
    emit contextMenuRequested(event->globalPos());
}

void TabButton::enterEvent(QEnterEvent *event) {
    m_hover = true;
    update();
    QWidget::enterEvent(event);
}

void TabButton::leaveEvent(QEvent *event) {
    m_hover = false;
    update();
    QWidget::leaveEvent(event);
}

/* ------------------------------------------------------------------ */
/* FileTabs                                                            */
/* ------------------------------------------------------------------ */

FileTabs::FileTabs(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("fileTabs"));
    setFixedHeight(kStripHeight);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setStyleSheet(QStringLiteral("QScrollArea { background-color: #232729; }"));

    m_container = new QWidget(m_scroll);
    m_container->setStyleSheet(QStringLiteral("background-color: #232729;"));
    m_layout = new QHBoxLayout(m_container);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addStretch(1);
    m_scroll->setWidget(m_container);

    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    outer->addWidget(m_scroll);
}

void FileTabs::setTabs(const QList<FileTabData> &tabs) {
    qDeleteAll(m_buttons);
    m_buttons.clear();

    for (const FileTabData &tab : tabs) {
        TabButton *button = new TabButton(m_container);
        button->setData(tab);
        button->setActive(m_buttons.size() == m_activeIndex);
        connect(button, &TabButton::clicked, this, [this, button] {
            const int index = m_buttons.indexOf(button);
            if (index >= 0) emit tabClicked(index);
        });
        connect(button, &TabButton::contextMenuRequested, this, [this, button](const QPoint &pos) {
            const int index = m_buttons.indexOf(button);
            if (index >= 0) emit tabContextMenu(index, pos);
        });
        m_layout->insertWidget(m_layout->count() - 1, button);
        m_buttons.append(button);
    }
    m_activeIndex = qBound(-1, m_activeIndex, m_buttons.size() - 1);
}

void FileTabs::setActiveIndex(int index) {
    m_activeIndex = index;
    for (int i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i]->setActive(i == index);
    }
}

int FileTabs::indexOf(const QString &id) const {
    for (int i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i]->data().id == id) return i;
    }
    return -1;
}

void FileTabs::ensureVisible(int index) {
    if (index < 0 || index >= m_buttons.size()) return;
    m_scroll->ensureWidgetVisible(m_buttons[index]);
}
