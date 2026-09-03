#include "welcometab.h"
#include "icons.h"
#include "lang.h"
#include "theme.h"

#include <QDesktopServices>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include <functional>

namespace {
const QUrl kWebsiteUrl = QUrl(QStringLiteral("https://acode.app"));
const QUrl kGitHubUrl = QUrl(QStringLiteral("https://github.com/Acode-Foundation/Acode"));
const QUrl kTelegramUrl = QUrl(QStringLiteral("https://t.me/foxdebug_acode"));
const QUrl kDiscordUrl = QUrl(QStringLiteral("https://discord.gg/nDqZsh7Rqz"));
} // namespace

WelcomeTab::WelcomeTab(QWidget *parent) : QScrollArea(parent) {
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_content = new QWidget(this);
    m_content->setStyleSheet(QStringLiteral("background-color: #2d3134;"));

    QVBoxLayout *layout = new QVBoxLayout(m_content);
    layout->setContentsMargins(24, 32, 24, 32);
    layout->setSpacing(0);

    /* hero */
    QWidget *hero = new QWidget(m_content);
    QHBoxLayout *heroLayout = new QHBoxLayout(hero);
    heroLayout->setContentsMargins(0, 0, 0, 0);
    heroLayout->setSpacing(16);

    QLabel *logo = new QLabel(hero);
    QPixmap logoPixmap(QStringLiteral(":/res/logo.png"));
    logo->setPixmap(logoPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setFixedSize(48, 48);

    QWidget *heroText = new QWidget(hero);
    QVBoxLayout *heroTextLayout = new QVBoxLayout(heroText);
    heroTextLayout->setContentsMargins(0, 0, 0, 0);
    heroTextLayout->setSpacing(4);

    m_titleLabel = new QLabel(heroText);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPixelSize(20);
    titleFont.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(QStringLiteral("color: #f5f5f5; letter-spacing: -0.3px;"));

    m_taglineLabel = new QLabel(heroText);
    QFont taglineFont = m_taglineLabel->font();
    taglineFont.setPixelSize(13);
    m_taglineLabel->setFont(taglineFont);
    m_taglineLabel->setStyleSheet(QStringLiteral("color: rgba(228, 228, 228, 0.6);"));

    heroTextLayout->addWidget(m_titleLabel);
    heroTextLayout->addWidget(m_taglineLabel);

    heroLayout->addWidget(logo);
    heroLayout->addWidget(heroText, 1);
    heroLayout->addStretch(1);

    layout->addWidget(hero);
    layout->addSpacing(48);

    auto addSection = [this, layout](const QString &label, const QList<QWidget *> &rows) {
        QLabel *sectionLabel = new QLabel(label, m_content);
        QFont font = sectionLabel->font();
        font.setPixelSize(11);
        font.setWeight(QFont::DemiBold);
        font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
        sectionLabel->setFont(font);
        sectionLabel->setStyleSheet(QStringLiteral(
            "color: rgba(228, 228, 228, 0.5); padding-left: 4px;"));
        m_sectionLabels.append(sectionLabel);
        layout->addWidget(sectionLabel);
        layout->addSpacing(12);
        for (QWidget *row : rows) {
            layout->addWidget(row);
        }
        layout->addSpacing(32);
    };

    /* GET STARTED */
    addSection(QStringLiteral("GET STARTED"), {
        makeActionRow(Icons::Add, Lang::s(QStringLiteral("new file")), [this] { emit newFileRequested(); }),
        makeActionRow(Icons::DocText, Lang::s(QStringLiteral("open file")), [this] { emit openFileRequested(); }),
        makeActionRow(Icons::FolderOpen, Lang::s(QStringLiteral("open folder")), [this] { emit openFolderRequested(); }),
        makeActionRow(Icons::Terminal, Lang::s(QStringLiteral("terminal")), [this] { emit openTerminalRequested(); }),
        makeActionRow(Icons::History, Lang::s(QStringLiteral("recent")), [this] { emit openRecentRequested(); }),
        makeActionRow(Icons::CommandPalette, Lang::s(QStringLiteral("command palette")), [this] { emit commandPaletteRequested(); }),
    });

    /* CONFIGURE */
    addSection(QStringLiteral("CONFIGURE"), {
        makeActionRow(Icons::Settings, Lang::s(QStringLiteral("settings")), [this] { emit openSettingsRequested(); }),
        makeActionRow(Icons::Extension, Lang::s(QStringLiteral("explore")) + QLatin1Char(' ') + Lang::s(QStringLiteral("plugins")), [this] { emit explorePluginsRequested(); }),
    });

    /* LEARN */
    addSection(QStringLiteral("LEARN"), {
        makeActionRow(Icons::Help, Lang::s(QStringLiteral("help")), [this] { emit helpRequested(); }),
        makeActionRow(Icons::Info, Lang::s(QStringLiteral("about")), [this] { emit aboutRequested(); }),
    });

    /* CONNECT */
    addSection(QStringLiteral("CONNECT"), {
        makeLinkRow(QStringLiteral("Website"), kWebsiteUrl),
        makeLinkRow(QStringLiteral("GitHub"), kGitHubUrl),
        makeLinkRow(QStringLiteral("Telegram"), kTelegramUrl),
        makeLinkRow(QStringLiteral("Discord"), kDiscordUrl),
    });

    layout->addStretch(1);

    setWidget(m_content);
    retranslate();
}

QWidget *WelcomeTab::makeActionRow(Icons::Icon icon, const QString &label, const std::function<void()> &handler) {
    QPushButton *button = new QPushButton(m_content);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedHeight(44);
    button->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 8px;"
        "  text-align: left;"
        "  color: #f5f5f5;"
        "  font-size: 15px;"
        "  padding: 0 12px;"
        "}"
        "QPushButton:hover { background-color: rgba(255, 255, 255, 0.06); }"
        "QPushButton:pressed { background-color: rgba(255, 255, 255, 0.1); }"));
    button->setIcon(Icons::icon(icon, Theme::PrimaryText));
    button->setIconSize(QSize(22, 22));
    button->setText(label);
    connect(button, &QPushButton::clicked, this, handler);
    return button;
}

QWidget *WelcomeTab::makeLinkRow(const QString &label, const QUrl &url) {
    QPushButton *button = new QPushButton(m_content);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedHeight(44);
    button->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 8px;"
        "  text-align: left;"
        "  color: #8ab4f8;"
        "  font-size: 15px;"
        "  padding: 0 12px;"
        "}"
        "QPushButton:hover { background-color: rgba(255, 255, 255, 0.06); }"));
    button->setIcon(Icons::icon(Icons::Globe, Theme::Link));
    button->setIconSize(QSize(22, 22));
    button->setText(label);
    connect(button, &QPushButton::clicked, this, [url] { QDesktopServices::openUrl(url); });
    return button;
}

void WelcomeTab::retranslate() {
    m_titleLabel->setText(QStringLiteral("Welcome to CIDE"));
    m_taglineLabel->setText(QStringLiteral("C code editor for Android"));
    setToolTip(QStringLiteral("Get Started"));
}
