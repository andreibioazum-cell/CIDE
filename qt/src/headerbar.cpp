#include "headerbar.h"
#include "icons.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QEvent>
#include <QPainter>
#include <QVBoxLayout>

/* ------------------------------------------------------------------ */
/* IconButton                                                          */
/* ------------------------------------------------------------------ */

IconButton::IconButton(QWidget *parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMouseTracking(true);
    setAttribute(Qt::WA_StyledBackground, false);
}

QSize IconButton::sizeHint() const {
    return QSize(45, 45);
}

void IconButton::setIcon(const QIcon &icon, int extent) {
    m_icon = icon;
    m_iconExtent = extent;
    m_text.clear();
    update();
}

void IconButton::setText(const QString &text, int pixelSize) {
    m_text = text;
    m_textSize = pixelSize;
    m_icon = QIcon();
    update();
}

void IconButton::setFontPixelSize(int size) {
    m_textSize = size;
    update();
}

void IconButton::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;
    update();
}

void IconButton::setBadge(bool badge) {
    if (m_badge == badge) return;
    m_badge = badge;
    update();
}

void IconButton::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_pressed || m_active) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(Theme::tabActiveBackground());
        painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 6, 6);
    } else if (m_hover) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 18));
        painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 6, 6);
    }

    const QColor color = Theme::PrimaryText;
    painter.setOpacity(isEnabled() ? 1.0 : 0.45); /* [disabled] opacity .45 */
    if (!m_icon.isNull()) {
        const QPixmap pixmap = m_icon.pixmap(QSize(m_iconExtent, m_iconExtent));
        if (!pixmap.isNull()) {
            painter.drawPixmap((width() - m_iconExtent) / 2, (height() - m_iconExtent) / 2, pixmap);
        }
    } else if (!m_text.isEmpty()) {
        QFont font = this->font();
        font.setBold(true);
        font.setPixelSize(m_textSize);
        painter.setFont(font);
        painter.setPen(color);
        painter.drawText(rect(), Qt::AlignCenter, m_text);
    }
    painter.setOpacity(1.0);

    if (m_badge) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(Theme::Active);
        painter.drawEllipse(QPointF(width() - 10, 10), 3.2, 3.2);
    }
}

void IconButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
    }
    QWidget::mousePressEvent(event);
}

void IconButton::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        update();
        if (rect().contains(event->pos())) emit clicked();
    }
    QWidget::mouseReleaseEvent(event);
}

void IconButton::mouseMoveEvent(QMouseEvent *event) {
    if (!m_hover) {
        m_hover = true;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void IconButton::leaveEvent(QEvent *event) {
    m_hover = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void IconButton::enterEvent(QEnterEvent *event) {
    m_hover = true;
    update();
    QWidget::enterEvent(event);
}

/* ------------------------------------------------------------------ */
/* HeaderBar                                                           */
/* ------------------------------------------------------------------ */

HeaderBar::HeaderBar(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("headerBar"));
    setFixedHeight(45); /* .header.tile height */

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_navButton = new IconButton(this);
    m_navButton->setIcon(Icons::icon(Icons::Menu));
    m_navButton->setToolTip(QStringLiteral("Toggle sidebar"));
    connect(m_navButton, &IconButton::clicked, this, &HeaderBar::navToggled);

    m_titleLabel = new QLabel(QStringLiteral("CIDE"), this);
    m_titleLabel->setObjectName(QStringLiteral("headerTitle"));
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPixelSize(17); /* 1.2em of 14px */
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_subLabel = new QLabel(this);
    QFont subFont = m_subLabel->font();
    subFont.setPixelSize(10); /* 0.58em */
    m_subLabel->setFont(subFont);
    m_subLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_subLabel->setStyleSheet(QStringLiteral("color: rgba(245, 245, 245, 0.6);"));

    QVBoxLayout *titleLayout = new QVBoxLayout;
    titleLayout->setContentsMargins(6, 4, 6, 4);
    titleLayout->setSpacing(0);
    titleLayout->addStretch(1);
    titleLayout->addWidget(m_titleLabel, 0, Qt::AlignVCenter);
    titleLayout->addWidget(m_subLabel, 0, Qt::AlignVCenter);
    titleLayout->addStretch(1);

    m_editButton = new IconButton(this);
    m_editButton->setIcon(Icons::icon(Icons::Edit));
    m_editButton->setVisible(false);
    connect(m_editButton, &IconButton::clicked, this, &HeaderBar::fileMenuToggled);

    m_runButton = new IconButton(this);
    m_runButton->setIcon(Icons::icon(Icons::PlayArrow));
    m_runButton->setVisible(false);
    connect(m_runButton, &IconButton::clicked, this, &HeaderBar::runClicked);

    m_moreButton = new IconButton(this);
    m_moreButton->setIcon(Icons::icon(Icons::MoreVert));
    connect(m_moreButton, &IconButton::clicked, this, &HeaderBar::menuToggled);

    m_layout->addWidget(m_navButton);
    m_layout->addLayout(titleLayout, 1);
    m_layout->addWidget(m_editButton);
    m_layout->addWidget(m_runButton);
    m_layout->addWidget(m_moreButton);
}

void HeaderBar::setTitle(const QString &title) {
    m_titleLabel->setText(title);
}

void HeaderBar::setSubTitle(const QString &subTitle) {
    m_subLabel->setText(subTitle);
    m_subLabel->setVisible(!subTitle.isEmpty());
}

void HeaderBar::setFileMenuVisible(bool visible) {
    m_editButton->setVisible(visible);
}

void HeaderBar::setRunVisible(bool visible) {
    m_runButton->setVisible(visible);
}
