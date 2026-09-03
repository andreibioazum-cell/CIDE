#include "codeeditor.h"
#include "appstate.h"
#include "theme.h"

#include <QAbstractTextDocumentLayout>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QPainter>
#include <QTextBlock>

/* ------------------------------------------------------------------ */
/* LineNumberArea                                                      */
/* ------------------------------------------------------------------ */

LineNumberArea::LineNumberArea(CodeEditor *editor) : QWidget(editor), m_editor(editor) {
}

QSize LineNumberArea::sizeHint() const {
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    m_editor->lineNumberAreaPaintEvent(event);
}

/* ------------------------------------------------------------------ */
/* CodeEditor                                                          */
/* ------------------------------------------------------------------ */

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setFrameShape(QFrame::NoFrame);
    setViewportMargins(0, 0, 0, 0);
    setCursorWidth(2); /* settings: cursorWidth 2 */

    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    applySettings();
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

QString CodeEditor::defaultFontFamily() {
    /* web default editorFont is "Roboto Mono" (src/lib/settings.js) */
    const QString preferred = QStringLiteral("Roboto Mono");
    const QStringList families = QFontDatabase::families();
    if (families.contains(preferred)) return preferred;
    for (const QString &candidate : {QStringLiteral("MesloLGS NF Regular"), QStringLiteral("Droid Sans Mono"), QStringLiteral("DejaVu Sans Mono"), QStringLiteral("Consolas"), QStringLiteral("Courier New")}) {
        if (families.contains(candidate)) return candidate;
    }
    return QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
}

void CodeEditor::applySettings() {
    AppState *state = AppState::instance();
    QFont font(defaultFontFamily());
    font.setStyleHint(QFont::Monospace);
    font.setPixelSize(qMax(8, state->fontSize()));
    setFont(font);
    setTabStopDistance(state->tabSize() * QFontMetrics(font).horizontalAdvance(QLatin1Char(' ')));
    setLineWrapMode(state->textWrap() ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    updateLineNumberAreaWidth(blockCount());
}

bool CodeEditor::isReadOnlyFile() const {
    return isReadOnly();
}

int CodeEditor::lineNumberAreaWidth() const {
    if (!AppState::instance()->lineNumbers()) return 0;
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    const int space = 8 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::focusInEvent(QFocusEvent *event) {
    emit editorFocus();
    QPlainTextEdit::focusInEvent(event);
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    AppState *state = AppState::instance();
    /* Tab / Backspace indentation like CodeMirror defaults */
    if (event->key() == Qt::Key_Tab && !textCursor().hasSelection()) {
        if (state->softTab()) {
            insertPlainText(QString(state->tabSize(), QLatin1Char(' ')));
            return;
        }
    } else if (event->key() == Qt::Key_Tab && textCursor().hasSelection()) {
        QTextCursor cursor = textCursor();
        const int start = cursor.selectionStart();
        const int end = cursor.selectionEnd();
        cursor.beginEditBlock();
        QTextBlock block = document()->findBlock(start);
        const QTextBlock lastBlock = document()->findBlock(end);
        while (block.isValid() && block.blockNumber() <= lastBlock.blockNumber()) {
            cursor.setPosition(block.position());
            if (state->softTab()) cursor.insertText(QString(state->tabSize(), QLatin1Char(' ')));
            else cursor.insertText(QStringLiteral("\t"));
            block = block.next();
        }
        cursor.endEditBlock();
        setTextCursor(cursor);
        return;
    } else if (event->key() == Qt::Key_Backtab && textCursor().hasSelection()) {
        QTextCursor cursor = textCursor();
        const int start = cursor.selectionStart();
        const int end = cursor.selectionEnd();
        cursor.beginEditBlock();
        QTextBlock block = document()->findBlock(start);
        const QTextBlock lastBlock = document()->findBlock(end);
        while (block.isValid() && block.blockNumber() <= lastBlock.blockNumber()) {
            cursor.setPosition(block.position());
            cursor.setPosition(block.position() + state->tabSize(), QTextCursor::KeepAnchor);
            QString text = cursor.selectedText();
            if (text.startsWith(QLatin1Char('\t')) || text.startsWith(QLatin1Char(' '))) {
                cursor.removeSelectedText();
            }
            block = block.next();
        }
        cursor.endEditBlock();
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(Theme::EditorActiveLine);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Theme::EditorBackground);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const QString number = QString::number(blockNumber + 1);
            painter.setPen(Theme::EditorGutter);
            painter.drawText(0, top, m_lineNumberArea->width() - 5,
                             fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int CodeEditor::currentLine() const {
    return textCursor().blockNumber() + 1;
}

void CodeEditor::gotoLine(int line) {
    const int target = qBound(1, line, blockCount());
    QTextBlock block = document()->findBlockByNumber(target - 1);
    if (!block.isValid()) return;
    QTextCursor cursor(block);
    setTextCursor(cursor);
    centerCursor();
    setFocus();
}

void CodeEditor::insertAtCursor(const QString &text) {
    insertPlainText(text);
    setFocus();
}

void CodeEditor::moveCurrentLineUp() {
    QTextCursor cursor = textCursor();
    const int blockNumber = cursor.blockNumber();
    if (blockNumber == 0) return;
    const QString currentText = cursor.block().text();
    const QString previousText = document()->findBlockByNumber(blockNumber - 1).text();
    const int positionInBlock = cursor.positionInBlock();

    QTextCursor edit(document());
    edit.beginEditBlock();
    const QTextBlock firstBlock = document()->findBlockByNumber(blockNumber - 1);
    const QTextBlock secondBlock = document()->findBlockByNumber(blockNumber);
    edit.setPosition(firstBlock.position());
    edit.setPosition(secondBlock.position() + secondBlock.length() - 1, QTextCursor::KeepAnchor);
    edit.insertText(currentText + QLatin1Char('\n') + previousText);
    edit.endEditBlock();

    QTextCursor fix(document());
    fix.setPosition(document()->findBlockByNumber(blockNumber - 1).position() + positionInBlock);
    setTextCursor(fix);
}

void CodeEditor::moveCurrentLineDown() {
    QTextCursor cursor = textCursor();
    const int blockNumber = cursor.blockNumber();
    if (blockNumber >= blockCount() - 1) return;
    const QString currentText = cursor.block().text();
    const QString nextText = document()->findBlockByNumber(blockNumber + 1).text();
    const int positionInBlock = cursor.positionInBlock();

    QTextCursor edit(document());
    edit.beginEditBlock();
    const QTextBlock firstBlock = document()->findBlockByNumber(blockNumber);
    const QTextBlock secondBlock = document()->findBlockByNumber(blockNumber + 1);
    edit.setPosition(firstBlock.position());
    edit.setPosition(secondBlock.position() + secondBlock.length() - 1, QTextCursor::KeepAnchor);
    edit.insertText(nextText + QLatin1Char('\n') + currentText);
    edit.endEditBlock();

    QTextCursor fix(document());
    fix.setPosition(document()->findBlockByNumber(blockNumber + 1).position() + positionInBlock);
    setTextCursor(fix);
}

void CodeEditor::copyLineUp() {
    const int blockNumber = textCursor().blockNumber();
    const QString text = textCursor().block().text();

    QTextCursor edit(document());
    edit.beginEditBlock();
    const QTextBlock block = document()->findBlockByNumber(blockNumber);
    edit.setPosition(block.position());
    edit.insertText(text + QLatin1Char('\n'));
    edit.endEditBlock();

    QTextCursor fix(document());
    fix.setPosition(document()->findBlockByNumber(blockNumber).position());
    setTextCursor(fix);
}

void CodeEditor::copyLineDown() {
    const QString text = textCursor().block().text();

    QTextCursor edit(document());
    edit.beginEditBlock();
    const QTextBlock block = textCursor().block();
    edit.setPosition(block.position() + block.length() - 1);
    edit.insertText(QLatin1Char('\n') + text);
    edit.endEditBlock();
}
