#pragma once

#include <QWidget>

class QHBoxLayout;
class QLabel;
class QLineEdit;
class QVBoxLayout;

class IconButton;

/*
 * Quick tools footer (components/quickTools): two rows of 16 keys built
 * from the default quicktoolsItems list in src/lib/settings.js at
 * commit 3673f5a, plus the in-file search rows (SearchRow1/SearchRow2).
 */
class QuickTools : public QWidget {
    Q_OBJECT

public:
    enum Mode { Hidden = 0, OneRow = 1, TwoRows = 2 };
    enum Modifier { Shift = 0, Ctrl = 1, Alt = 2, MaxModifiers = 3 };

    explicit QuickTools(QWidget *parent = nullptr);

    void setMode(Mode mode);
    Mode mode() const { return m_mode; }

    void setSearchMode(bool searchMode);
    bool searchMode() const { return m_searchMode; }

    void setSaveBadge(bool unsaved);
    void setCanUndo(bool can);
    void setCanRedo(bool can);
    void updateSearchStatus(int position, int total);
    bool modifierActive(Modifier modifier) const;
    void retranslate();

signals:
    void saveRequested();
    void undoRequested();
    void redoRequested();
    void searchToggled();
    void searchClosed();
    void searchNextRequested(const QString &term, bool matchCase);
    void searchPrevRequested(const QString &term, bool matchCase);
    void replaceNextRequested(const QString &term, const QString &replacement, bool matchCase);
    void replaceAllRequested(const QString &term, const QString &replacement, bool matchCase);
    void matchCaseToggled();
    void textInsertRequested(const QString &text);
    void tabRequested();
    void escRequested();
    void arrowRequested(int key, bool shift, bool ctrl);
    void moveLineUpRequested();
    void moveLineDownRequested();
    void copyLineUpRequested();
    void copyLineDownRequested();
    void pasteRequested();
    void selectAllRequested();
    void modeChanged();

private:
    void buildUi();
    QWidget *buildRow(int row);
    QWidget *buildSearchRow1();
    QWidget *buildSearchRow2();
    IconButton *makeKey(const QIcon &icon, const QString &tooltipKey);
    IconButton *makeLetterKey(const QString &text, const QString &tooltipKey);
    void syncModifiers();

    Mode m_mode = TwoRows;
    bool m_searchMode = false;
    bool m_matchCase = false;
    bool m_modifierState[MaxModifiers] = {false, false, false};

    QVBoxLayout *m_rowsLayout = nullptr;
    QWidget *m_row1 = nullptr;
    QWidget *m_row2 = nullptr;
    QWidget *m_searchRow1 = nullptr;
    QWidget *m_searchRow2 = nullptr;
    IconButton *m_toggler = nullptr;
    IconButton *m_saveKey = nullptr;
    IconButton *m_undoKey = nullptr;
    IconButton *m_redoKey = nullptr;
    IconButton *m_shiftKey = nullptr;
    IconButton *m_ctrlKey = nullptr;
    IconButton *m_altKey = nullptr;
    IconButton *m_matchCaseKey = nullptr;
    QLineEdit *m_searchInput = nullptr;
    QLineEdit *m_replaceInput = nullptr;
    QLabel *m_searchStatus = nullptr;
};
