#include "quicktools.h"
#include "appstate.h"
#include "headerbar.h"
#include "icons.h"
#include "lang.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace {
constexpr int kKeySize = 40; /* .section min-height 40 */
} // namespace

QuickTools::QuickTools(QWidget *parent) : QWidget(parent) {
    setStyleSheet(QStringLiteral("background-color: #232729;"));
    buildUi();
    setMode(TwoRows);
}

IconButton *QuickTools::makeKey(const QIcon &icon, const QString &tooltipKey) {
    IconButton *button = new IconButton(this);
    button->setFixedSize(kKeySize, kKeySize);
    button->setIcon(icon, 20);
    if (!tooltipKey.isEmpty()) button->setToolTip(Lang::s(tooltipKey));
    return button;
}

IconButton *QuickTools::makeLetterKey(const QString &text, const QString &tooltipKey) {
    IconButton *button = new IconButton(this);
    button->setFixedSize(kKeySize, kKeySize);
    button->setText(text, 11);
    if (!tooltipKey.isEmpty()) button->setToolTip(Lang::s(tooltipKey));
    return button;
}

void QuickTools::buildUi() {
    m_row1 = buildRow(1);
    m_row2 = buildRow(2);
    m_searchRow1 = buildSearchRow1();
    m_searchRow2 = buildSearchRow2();
    m_row1->hide();
    m_row2->hide();
    m_searchRow1->hide();
    m_searchRow2->hide();

    QWidget *rowsHost = new QWidget(this);
    rowsHost->setStyleSheet(QStringLiteral("background-color: #232729;"));
    m_rowsLayout = new QVBoxLayout(rowsHost);
    m_rowsLayout->setContentsMargins(0, 0, 0, 0);
    m_rowsLayout->setSpacing(0);

    QHBoxLayout *outer = new QHBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    outer->addWidget(rowsHost, 1);

    m_toggler = new IconButton(this);
    m_toggler->setFixedSize(28, kKeySize);
    m_toggler->setIcon(Icons::icon(Icons::ChevronUp, QColor(245, 245, 245, 180)), 18);
    connect(m_toggler, &IconButton::clicked, this, [this] {
        if (m_searchMode) {
            setSearchMode(false);
        } else if (m_mode == TwoRows) {
            setMode(OneRow);
        } else if (m_mode == OneRow) {
            setMode(Hidden);
        } else {
            setMode(TwoRows);
        }
    });
    outer->addWidget(m_toggler);
}

QWidget *QuickTools::buildRow(int row) {
    QWidget *widget = new QWidget(this);
    widget->setStyleSheet(QStringLiteral("background-color: #232729;"));
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto insertChar = [this, layout](const QString &text, const QString &tooltipKey) {
        IconButton *button = makeLetterKey(text, tooltipKey);
        connect(button, &IconButton::clicked, this, [this, text] { emit textInsertRequested(text); });
        layout->addWidget(button);
    };

    if (row == 1) {
        /* shift, tab, search, undo, redo, up, save, esc, { } [ ] ( ) < > */
        m_shiftKey = makeLetterKey(QStringLiteral("shft"), QStringLiteral("quicktools:shift-key"));
        connect(m_shiftKey, &IconButton::clicked, this, [this] {
            m_modifierState[Shift] = !m_modifierState[Shift];
            syncModifiers();
        });
        layout->addWidget(m_shiftKey);

        IconButton *tabKey = makeKey(Icons::icon(Icons::KeyboardTab), QStringLiteral("quicktools:tab-key"));
        connect(tabKey, &IconButton::clicked, this, [this] { emit tabRequested(); });
        layout->addWidget(tabKey);

        IconButton *searchKey = makeKey(Icons::icon(Icons::Search), QStringLiteral("quicktools:search"));
        connect(searchKey, &IconButton::clicked, this, [this] { setSearchMode(!m_searchMode); });
        layout->addWidget(searchKey);

        m_undoKey = makeKey(Icons::icon(Icons::Undo), QStringLiteral("quicktools:undo"));
        connect(m_undoKey, &IconButton::clicked, this, [this] { emit undoRequested(); });
        layout->addWidget(m_undoKey);

        m_redoKey = makeKey(Icons::icon(Icons::Redo), QStringLiteral("quicktools:redo"));
        connect(m_redoKey, &IconButton::clicked, this, [this] { emit redoRequested(); });
        layout->addWidget(m_redoKey);

        IconButton *upKey = makeKey(Icons::icon(Icons::ArrowUp), QStringLiteral("quicktools:up-arrow-key"));
        connect(upKey, &IconButton::clicked, this, [this] {
            emit arrowRequested(int(Qt::Key_Up), m_modifierState[Shift], m_modifierState[Ctrl]);
        });
        layout->addWidget(upKey);

        m_saveKey = makeKey(Icons::icon(Icons::Save), QStringLiteral("quicktools:save"));
        connect(m_saveKey, &IconButton::clicked, this, [this] { emit saveRequested(); });
        layout->addWidget(m_saveKey);

        IconButton *escKey = makeLetterKey(QStringLiteral("esc"), QStringLiteral("quicktools:esc-key"));
        connect(escKey, &IconButton::clicked, this, [this] { emit escRequested(); });
        layout->addWidget(escKey);

        insertChar(QStringLiteral("{"), QStringLiteral("quicktools:curlybracket"));
        insertChar(QStringLiteral("}"), QStringLiteral("quicktools:curlybracket"));
        insertChar(QStringLiteral("["), QStringLiteral("quicktools:squarebracket"));
        insertChar(QStringLiteral("]"), QStringLiteral("quicktools:squarebracket"));
        insertChar(QStringLiteral("("), QStringLiteral("quicktools:parentheses"));
        insertChar(QStringLiteral(")"), QStringLiteral("quicktools:parentheses"));
        insertChar(QStringLiteral("<"), QStringLiteral("quicktools:anglebracket"));
        insertChar(QStringLiteral(">"), QStringLiteral("quicktools:anglebracket"));
    } else {
        /* ctrl, alt, moveline-down, moveline-up, left, down, right,
           copyline-down, ; ' " & | = / ! */
        m_ctrlKey = makeLetterKey(QStringLiteral("ctrl"), QStringLiteral("quicktools:ctrl-key"));
        connect(m_ctrlKey, &IconButton::clicked, this, [this] {
            m_modifierState[Ctrl] = !m_modifierState[Ctrl];
            syncModifiers();
        });
        layout->addWidget(m_ctrlKey);

        m_altKey = makeLetterKey(QStringLiteral("alt"), QStringLiteral("quicktools:alt-key"));
        connect(m_altKey, &IconButton::clicked, this, [this] {
            m_modifierState[Alt] = !m_modifierState[Alt];
            syncModifiers();
        });
        layout->addWidget(m_altKey);

        IconButton *moveDown = makeKey(Icons::icon(Icons::MoveLineDown), QStringLiteral("quicktools:moveline-down"));
        connect(moveDown, &IconButton::clicked, this, [this] { emit moveLineDownRequested(); });
        layout->addWidget(moveDown);

        IconButton *moveUp = makeKey(Icons::icon(Icons::MoveLineUp), QStringLiteral("quicktools:moveline-up"));
        connect(moveUp, &IconButton::clicked, this, [this] { emit moveLineUpRequested(); });
        layout->addWidget(moveUp);

        IconButton *leftKey = makeKey(Icons::icon(Icons::ArrowLeft), QStringLiteral("quicktools:left-arrow-key"));
        connect(leftKey, &IconButton::clicked, this, [this] {
            emit arrowRequested(int(Qt::Key_Left), m_modifierState[Shift], m_modifierState[Ctrl]);
        });
        layout->addWidget(leftKey);

        IconButton *downKey = makeKey(Icons::icon(Icons::ArrowDown), QStringLiteral("quicktools:down-arrow-key"));
        connect(downKey, &IconButton::clicked, this, [this] {
            emit arrowRequested(int(Qt::Key_Down), m_modifierState[Shift], m_modifierState[Ctrl]);
        });
        layout->addWidget(downKey);

        IconButton *rightKey = makeKey(Icons::icon(Icons::ArrowRight), QStringLiteral("quicktools:right-arrow-key"));
        connect(rightKey, &IconButton::clicked, this, [this] {
            emit arrowRequested(int(Qt::Key_Right), m_modifierState[Shift], m_modifierState[Ctrl]);
        });
        layout->addWidget(rightKey);

        IconButton *copyDown = makeKey(Icons::icon(Icons::CopyLineDown), QStringLiteral("quicktools:copyline-down"));
        connect(copyDown, &IconButton::clicked, this, [this] { emit copyLineDownRequested(); });
        layout->addWidget(copyDown);

        insertChar(QStringLiteral(";"), QStringLiteral("quicktools:semicolon"));
        insertChar(QStringLiteral("'"), QStringLiteral("quicktools:quotation"));
        insertChar(QStringLiteral("\""), QStringLiteral("quicktools:quotation"));
        insertChar(QStringLiteral("&"), QStringLiteral("quicktools:and"));
        insertChar(QStringLiteral("|"), QStringLiteral("quicktools:bar"));
        insertChar(QStringLiteral("="), QStringLiteral("quicktools:equal"));
        insertChar(QStringLiteral("/"), QStringLiteral("quicktools:slash"));
        insertChar(QStringLiteral("!"), QStringLiteral("quicktools:exclamation"));
    }

    layout->addStretch(1);
    return widget;
}

QWidget *QuickTools::buildSearchRow1() {
    QWidget *widget = new QWidget(this);
    widget->setStyleSheet(QStringLiteral("background-color: #232729;"));
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_searchInput = new QLineEdit(widget);
    m_searchInput->setPlaceholderText(Lang::s(QStringLiteral("search")));
    connect(m_searchInput, &QLineEdit::returnPressed, this, [this] {
        emit searchNextRequested(m_searchInput->text(), m_matchCase);
    });

    IconButton *prev = makeKey(Icons::icon(Icons::ArrowLeft), QStringLiteral("quicktools:search-prev"));
    connect(prev, &IconButton::clicked, this, [this] { emit searchPrevRequested(m_searchInput->text(), m_matchCase); });
    IconButton *next = makeKey(Icons::icon(Icons::ArrowRight), QStringLiteral("quicktools:search-next"));
    connect(next, &IconButton::clicked, this, [this] { emit searchNextRequested(m_searchInput->text(), m_matchCase); });

    m_matchCaseKey = makeLetterKey(QStringLiteral("Aa"), QString());
    connect(m_matchCaseKey, &IconButton::clicked, this, [this] {
        m_matchCase = !m_matchCase;
        m_matchCaseKey->setActive(m_matchCase);
    });

    IconButton *close = makeKey(Icons::icon(Icons::Close), QStringLiteral("close"));
    connect(close, &IconButton::clicked, this, [this] { setSearchMode(false); });

    layout->addWidget(m_searchInput, 1);
    layout->addWidget(prev);
    layout->addWidget(next);
    layout->addWidget(m_matchCaseKey);
    layout->addWidget(close);
    return widget;
}

QWidget *QuickTools::buildSearchRow2() {
    QWidget *widget = new QWidget(this);
    widget->setStyleSheet(QStringLiteral("background-color: #232729;"));
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_replaceInput = new QLineEdit(widget);
    m_replaceInput->setPlaceholderText(Lang::s(QStringLiteral("replace")));

    IconButton *replaceNext = makeKey(Icons::icon(Icons::SubdirArrowLeft), QStringLiteral("quicktools:search-replace"));
    connect(replaceNext, &IconButton::clicked, this, [this] {
        emit replaceNextRequested(m_searchInput->text(), m_replaceInput->text(), m_matchCase);
    });
    IconButton *replaceAll = makeKey(Icons::icon(Icons::CompareArrows), QStringLiteral("quicktools:search-replace-all"));
    connect(replaceAll, &IconButton::clicked, this, [this] {
        emit replaceAllRequested(m_searchInput->text(), m_replaceInput->text(), m_matchCase);
    });

    m_searchStatus = new QLabel(QStringLiteral("0 of 0"), widget);
    m_searchStatus->setStyleSheet(QStringLiteral("color: rgba(245,245,245,0.7); font-size: 12px; padding: 0 6px;"));

    layout->addWidget(m_replaceInput, 1);
    layout->addWidget(replaceNext);
    layout->addWidget(replaceAll);
    layout->addWidget(m_searchStatus);
    return widget;
}

void QuickTools::setMode(Mode mode) {
    m_mode = mode;
    AppState::instance()->setQuickToolsSearchMode(false);

    /* remove all rows */
    while (m_rowsLayout->count()) {
        QLayoutItem *item = m_rowsLayout->takeAt(0);
        if (item->widget()) item->widget()->hide();
        delete item;
    }

    if (m_mode == TwoRows) {
        m_row1->show();
        m_row2->show();
        m_rowsLayout->addWidget(m_row1);
        m_rowsLayout->addWidget(m_row2);
    } else if (m_mode == OneRow) {
        m_row1->show();
        m_row2->hide();
        m_rowsLayout->addWidget(m_row1);
    } else {
        m_row1->hide();
        m_row2->hide();
    }

    m_toggler->setIcon(Icons::icon(m_mode == TwoRows ? Icons::ChevronUp : Icons::ChevronDown,
                                   QColor(245, 245, 245, 180)), 18);
    emit modeChanged();
}

void QuickTools::setSearchMode(bool searchMode) {
    if (m_searchMode == searchMode) return;
    m_searchMode = searchMode;
    AppState::instance()->setQuickToolsSearchMode(searchMode);
    m_mode = TwoRows; /* ensure footer visible after closing search */

    while (m_rowsLayout->count()) {
        QLayoutItem *item = m_rowsLayout->takeAt(0);
        if (item->widget()) item->widget()->hide();
        delete item;
    }

    if (m_searchMode) {
        m_searchRow1->show();
        m_searchRow2->show();
        m_rowsLayout->addWidget(m_searchRow1);
        m_rowsLayout->addWidget(m_searchRow2);
        m_searchInput->setFocus();
        m_toggler->setIcon(Icons::icon(Icons::ChevronDown, QColor(245, 245, 245, 180)), 18);
        emit searchToggled();
    } else {
        m_searchRow1->hide();
        m_searchRow2->hide();
        setMode(TwoRows);
    }
}

void QuickTools::setSaveBadge(bool unsaved) {
    if (m_saveKey) m_saveKey->setBadge(unsaved);
}

void QuickTools::setCanUndo(bool can) {
    if (m_undoKey) m_undoKey->setEnabled(can);
}

void QuickTools::setCanRedo(bool can) {
    if (m_redoKey) m_redoKey->setEnabled(can);
}

void QuickTools::updateSearchStatus(int position, int total) {
    if (m_searchStatus) {
        m_searchStatus->setText(QStringLiteral("%1 of %2").arg(position).arg(total));
    }
}

bool QuickTools::modifierActive(Modifier modifier) const {
    return m_modifierState[modifier];
}

void QuickTools::syncModifiers() {
    if (m_shiftKey) m_shiftKey->setActive(m_modifierState[Shift]);
    if (m_ctrlKey) m_ctrlKey->setActive(m_modifierState[Ctrl]);
    if (m_altKey) m_altKey->setActive(m_modifierState[Alt]);
}

void QuickTools::retranslate() {
    if (m_searchInput) m_searchInput->setPlaceholderText(Lang::s(QStringLiteral("search")));
    if (m_replaceInput) m_replaceInput->setPlaceholderText(Lang::s(QStringLiteral("replace")));
}
