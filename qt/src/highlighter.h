#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

/*
 * Syntax highlighter using the CodeMirror "one_dark" palette
 * (the locked editor theme of CIDE at commit 3673f5a).
 */
class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    enum Language {
        PlainText,
        JavaScript,
        TypeScript,
        Json,
        Css,
        Scss,
        Html,
        Xml,
        Markdown,
        Python,
        C,
        Cpp,
        Java,
        Php,
        Shell,
        Yaml
    };

    static Language languageForFile(const QString &fileName);
    static QString languageName(Language language);
    static QStringList languageNames();
    static Language languageFromName(const QString &name);

    explicit Highlighter(QTextDocument *document, Language language = PlainText, QObject *parent = nullptr);
    void setLanguage(Language language);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
        int group;
    };

    void buildRules();
    void addRule(const QString &pattern, const QTextCharFormat &format, int group = 0);
    void applyRules(const QString &text);
    void markTaken(int start, int count);
    bool isTaken(int start, int count) const;

    void highlightCStyle(const QString &text);
    void highlightPython(const QString &text);
    void highlightMarkup(const QString &text);

    Language m_language;
    QVector<Rule> m_rules;
    QVector<QPair<int, int>> m_taken;

    QTextCharFormat m_fmtKeyword;
    QTextCharFormat m_fmtString;
    QTextCharFormat m_fmtNumber;
    QTextCharFormat m_fmtComment;
    QTextCharFormat m_fmtFunction;
    QTextCharFormat m_fmtVariable;
    QTextCharFormat m_fmtAttribute;
    QTextCharFormat m_fmtMeta;
};
