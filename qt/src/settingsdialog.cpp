#include "settingsdialog.h"
#include "appstate.h"
#include "lang.h"
#include "theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {
enum Page { MainListPage = 0, AppSettingsPage, EditorSettingsPage, TerminalSettingsPage, AboutPage };
} // namespace

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(Lang::s(QStringLiteral("settings")));
    setMinimumSize(QSize(420, 520));

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createMainPage());
    m_stack->addWidget(createAppSettingsPage());
    m_stack->addWidget(createEditorSettingsPage());
    m_stack->addWidget(createTerminalSettingsPage());
    m_stack->addWidget(createAboutPage());

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(Lang::s(QStringLiteral("ok")));
    buttons->button(QDialogButtonBox::Cancel)->setText(Lang::s(QStringLiteral("cancel")));
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        /* apply */
        AppState *state = AppState::instance();
        const QString oldLang = state->langCode();
        const QString newLang = m_languageNames.isEmpty() ? oldLang
            : (m_languageCombo->currentIndex() == 1 ? QStringLiteral("ru-ru") : QStringLiteral("en-us"));
        state->setLangCode(newLang);
        state->setFullscreen(m_fullscreenCheck->isChecked());
        state->setQuickToolsEnabled(m_quickToolsCheck->isChecked());
        state->setRememberFiles(m_rememberFilesCheck->isChecked());
        state->setRememberFolders(m_rememberFoldersCheck->isChecked());
        state->setFontSize(m_fontSizeSpin->value());
        state->setTabSize(m_tabSizeSpin->value());
        state->setSoftTab(m_softTabCheck->isChecked());
        state->setTextWrap(m_wrapCheck->isChecked());
        state->setLineNumbers(m_lineNumbersCheck->isChecked());
        if (oldLang != newLang) emit languageChanged();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 12);
    layout->setSpacing(12);
    layout->addWidget(m_stack, 1);
    layout->addWidget(buttons);
}

QWidget *SettingsDialog::createRow(Icons::Icon icon, const QString &text, const QString &info, const std::function<void()> &onActivate) {
    QPushButton *button = new QPushButton(this);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedHeight(56);
    button->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #2d3134;"
        "  border: none;"
        "  border-radius: 8px;"
        "  color: #f5f5f5;"
        "  padding: 0;"
        "}"
        "QPushButton:hover { background-color: #33383c; }"));

    QWidget *content = new QWidget(button);
    QHBoxLayout *layout = new QHBoxLayout(content);
    layout->setContentsMargins(14, 8, 14, 8);
    layout->setSpacing(14);

    QLabel *iconLabel = new QLabel(content);
    iconLabel->setPixmap(Icons::icon(icon, Theme::PrimaryText).pixmap(QSize(24, 24)));
    iconLabel->setFixedSize(24, 24);

    QLabel *textLabel = new QLabel(text, content);
    textLabel->setStyleSheet(QStringLiteral("color: #f5f5f5; font-size: 15px; background: transparent;"));

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel, 1);
    if (!info.isEmpty()) {
        QLabel *infoLabel = new QLabel(info, content);
        infoLabel->setWordWrap(true);
        infoLabel->setStyleSheet(QStringLiteral("color: rgba(228,228,228,0.55); font-size: 12px; background: transparent;"));
        layout->addWidget(infoLabel, 2);
    }

    QVBoxLayout *buttonLayout = new QVBoxLayout(button);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(content);

    connect(button, &QPushButton::clicked, this, onActivate);
    return button;
}

QWidget *SettingsDialog::createMainPage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    QLabel *title = new QLabel(Lang::s(QStringLiteral("settings")).toUpper(), page);
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);
    layout->addSpacing(8);

    layout->addWidget(createRow(Icons::Tune, Lang::s(QStringLiteral("app settings")), QString(), [this] { openPage(AppSettingsPage); }));
    layout->addWidget(createRow(Icons::TextFormat, Lang::s(QStringLiteral("editor settings")), QString(), [this] { openPage(EditorSettingsPage); }));
    layout->addWidget(createRow(Icons::Terminal, Lang::s(QStringLiteral("terminal settings")), QString(), [this] { openPage(TerminalSettingsPage); }));
    layout->addWidget(createRow(Icons::Info, Lang::s(QStringLiteral("about")), QString(), [this] { openPage(AboutPage); }));
    layout->addStretch(1);
    return page;
}

QWidget *SettingsDialog::createAppSettingsPage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    QLabel *title = new QLabel(Lang::s(QStringLiteral("app settings")), page);
    QFont titleFont = title->font();
    titleFont.setPixelSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);
    layout->addSpacing(8);

    QLabel *langTitle = new QLabel(Lang::s(QStringLiteral("language")), page);
    langTitle->setStyleSheet(QStringLiteral("font-size: 14px;"));
    m_languageNames = Lang::languageNames();
    m_languageCombo = new QComboBox(page);
    m_languageCombo->addItems(m_languageNames);
    m_languageCombo->setCurrentIndex(Lang::current() == Lang::Russian ? 1 : 0);

    m_fullscreenCheck = new QCheckBox(Lang::s(QStringLiteral("fullscreen")), page);
    m_fullscreenCheck->setChecked(AppState::instance()->fullscreen());

    m_quickToolsCheck = new QCheckBox(Lang::s(QStringLiteral("quick tools")), page);
    m_quickToolsCheck->setChecked(AppState::instance()->quickToolsEnabled());

    m_rememberFilesCheck = new QCheckBox(Lang::s(QStringLiteral("remember opened files")), page);
    m_rememberFilesCheck->setChecked(AppState::instance()->rememberFiles());

    m_rememberFoldersCheck = new QCheckBox(Lang::s(QStringLiteral("remember opened folders")), page);
    m_rememberFoldersCheck->setChecked(AppState::instance()->rememberFolders());

    QPushButton *back = new QPushButton(Lang::s(QStringLiteral("settings")), page);
    back->setFixedHeight(36);
    connect(back, &QPushButton::clicked, this, [this] { openPage(MainListPage); });

    layout->addWidget(langTitle);
    layout->addWidget(m_languageCombo);
    layout->addWidget(m_fullscreenCheck);
    layout->addWidget(m_quickToolsCheck);
    layout->addWidget(m_rememberFilesCheck);
    layout->addWidget(m_rememberFoldersCheck);
    layout->addStretch(1);
    layout->addWidget(back);
    return page;
}

QWidget *SettingsDialog::createEditorSettingsPage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    QLabel *title = new QLabel(Lang::s(QStringLiteral("editor settings")), page);
    QFont titleFont = title->font();
    titleFont.setPixelSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);
    layout->addSpacing(8);

    QLabel *fontLabel = new QLabel(Lang::s(QStringLiteral("font size")), page);
    m_fontSizeSpin = new QSpinBox(page);
    m_fontSizeSpin->setRange(8, 32);
    m_fontSizeSpin->setValue(AppState::instance()->fontSize());

    QLabel *tabLabel = new QLabel(Lang::s(QStringLiteral("tab size")), page);
    m_tabSizeSpin = new QSpinBox(page);
    m_tabSizeSpin->setRange(1, 8);
    m_tabSizeSpin->setValue(AppState::instance()->tabSize());

    m_softTabCheck = new QCheckBox(Lang::s(QStringLiteral("soft tab")), page);
    m_softTabCheck->setChecked(AppState::instance()->softTab());

    m_wrapCheck = new QCheckBox(Lang::s(QStringLiteral("text wrap")), page);
    m_wrapCheck->setChecked(AppState::instance()->textWrap());

    m_lineNumbersCheck = new QCheckBox(Lang::s(QStringLiteral("show line numbers")), page);
    m_lineNumbersCheck->setChecked(AppState::instance()->lineNumbers());

    QPushButton *back = new QPushButton(Lang::s(QStringLiteral("settings")), page);
    back->setFixedHeight(36);
    connect(back, &QPushButton::clicked, this, [this] { openPage(MainListPage); });

    layout->addWidget(fontLabel);
    layout->addWidget(m_fontSizeSpin);
    layout->addWidget(tabLabel);
    layout->addWidget(m_tabSizeSpin);
    layout->addWidget(m_softTabCheck);
    layout->addWidget(m_wrapCheck);
    layout->addWidget(m_lineNumbersCheck);
    layout->addStretch(1);
    layout->addWidget(back);
    return page;
}

QWidget *SettingsDialog::createTerminalSettingsPage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    QLabel *title = new QLabel(Lang::s(QStringLiteral("terminal settings")), page);
    QFont titleFont = title->font();
    titleFont.setPixelSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);
    layout->addSpacing(8);

    QLabel *note = new QLabel(page);
    note->setWordWrap(true);
    note->setStyleSheet(QStringLiteral("color: rgba(228,228,228,0.6); font-size: 13px;"));
    note->setText(QStringLiteral("Terminal is provided by the Android (Cordova) build.\n"
                                 "The Qt shell does not include a terminal."));

    QPushButton *back = new QPushButton(Lang::s(QStringLiteral("settings")), page);
    back->setFixedHeight(36);
    connect(back, &QPushButton::clicked, this, [this] { openPage(MainListPage); });

    layout->addWidget(note);
    layout->addStretch(1);
    layout->addWidget(back);
    return page;
}

QWidget *SettingsDialog::createAboutPage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 24, 16, 16);
    layout->setSpacing(10);

    QLabel *logo = new QLabel(page);
    QPixmap pixmap(QStringLiteral(":/res/logo.png"));
    logo->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setAlignment(Qt::AlignHCenter);

    QLabel *name = new QLabel(QStringLiteral("CIDE"), page);
    QFont nameFont = name->font();
    nameFont.setPixelSize(22);
    nameFont.setBold(true);
    name->setFont(nameFont);
    name->setAlignment(Qt::AlignHCenter);

    QLabel *version = new QLabel(QStringLiteral("1.13.2"), page);
    version->setAlignment(Qt::AlignHCenter);
    version->setStyleSheet(QStringLiteral("color: rgba(228,228,228,0.6);"));

    QLabel *description = new QLabel(QStringLiteral("CIDE - Android code editor"), page);
    description->setAlignment(Qt::AlignHCenter);
    description->setStyleSheet(QStringLiteral("color: rgba(228,228,228,0.6);"));

    QLabel *license = new QLabel(QStringLiteral("MIT License"), page);
    license->setAlignment(Qt::AlignHCenter);
    license->setStyleSheet(QStringLiteral("color: rgba(228,228,228,0.6);"));

    QPushButton *back = new QPushButton(Lang::s(QStringLiteral("settings")), page);
    back->setFixedHeight(36);
    connect(back, &QPushButton::clicked, this, [this] { openPage(MainListPage); });

    layout->addWidget(logo);
    layout->addWidget(name);
    layout->addWidget(version);
    layout->addWidget(description);
    layout->addWidget(license);
    layout->addStretch(1);
    layout->addWidget(back);
    return page;
}

void SettingsDialog::openPage(int index) {
    m_stack->setCurrentIndex(index);
}
