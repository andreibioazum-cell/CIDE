#pragma once

#include <QPlainTextEdit>
#include <QWidget>

class CodeEditor;

/*
 * Line number gutter drawn like CodeMirror's one_dark theme.
 */
class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *m_editor;
};

/*
 * Plain-text editor styled as the CodeMirror 6 "one_dark" editor used by
 * the web interface at commit 3673f5a.
 */
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

    void applySettings();          /* font size / tab / wrap / gutter settings */
    bool isReadOnlyFile() const;

    void moveCurrentLineUp();
    void moveCurrentLineDown();
    void copyLineUp();
    void copyLineDown();
    void insertAtCursor(const QString &text);
    void gotoLine(int line);
    int currentLine() const;

    static QString defaultFontFamily();

signals:
    void editorFocus();
    /* note: QPlainTextEdit::modificationChanged(bool) is used directly */

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *m_lineNumberArea;
};
