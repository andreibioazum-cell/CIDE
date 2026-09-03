#include "highlighter.h"
#include "theme.h"

#include <QFileInfo>

Highlighter::Highlighter(QTextDocument *document, Language language, QObject *parent)
    : QSyntaxHighlighter(document), m_language(language) {
    Q_UNUSED(parent);

    m_fmtKeyword.setForeground(Theme::EditorKeyword);
    m_fmtString.setForeground(Theme::EditorString);
    m_fmtNumber.setForeground(Theme::EditorNumber);
    m_fmtComment.setForeground(Theme::EditorComment);
    m_fmtComment.setFontItalic(true);
    m_fmtFunction.setForeground(Theme::EditorFunction);
    m_fmtVariable.setForeground(Theme::EditorVariable);
    m_fmtAttribute.setForeground(Theme::EditorAttribute);
    m_fmtMeta.setForeground(Theme::EditorOperator);

    buildRules();
}

/* ------------------------------------------------------------------ */
/* Language detection                                                  */
/* ------------------------------------------------------------------ */

Highlighter::Language Highlighter::languageForFile(const QString &fileName) {
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    if (suffix == QLatin1String("js") || suffix == QLatin1String("mjs") || suffix == QLatin1String("jsx")) return JavaScript;
    if (suffix == QLatin1String("ts") || suffix == QLatin1String("tsx")) return TypeScript;
    if (suffix == QLatin1String("json")) return Json;
    if (suffix == QLatin1String("css")) return Css;
    if (suffix == QLatin1String("scss") || suffix == QLatin1String("sass") || suffix == QLatin1String("less")) return Scss;
    if (suffix == QLatin1String("html") || suffix == QLatin1String("htm") || suffix == QLatin1String("vue")) return Html;
    if (suffix == QLatin1String("xml") || suffix == QLatin1String("svg")) return Xml;
    if (suffix == QLatin1String("md") || suffix == QLatin1String("markdown")) return Markdown;
    if (suffix == QLatin1String("py") || suffix == QLatin1String("pyw")) return Python;
    if (suffix == QLatin1String("c") || suffix == QLatin1String("h")) return C;
    if (suffix == QLatin1String("cpp") || suffix == QLatin1String("cc") || suffix == QLatin1String("cxx") || suffix == QLatin1String("hpp")) return Cpp;
    if (suffix == QLatin1String("java")) return Java;
    if (suffix == QLatin1String("php")) return Php;
    if (suffix == QLatin1String("sh") || suffix == QLatin1String("bash") || suffix == QLatin1String("zsh")) return Shell;
    if (suffix == QLatin1String("yml") || suffix == QLatin1String("yaml")) return Yaml;
    return PlainText;
}

QString Highlighter::languageName(Language language) {
    switch (language) {
    case JavaScript: return QStringLiteral("JavaScript");
    case TypeScript: return QStringLiteral("TypeScript");
    case Json: return QStringLiteral("JSON");
    case Css: return QStringLiteral("CSS");
    case Scss: return QStringLiteral("SCSS");
    case Html: return QStringLiteral("HTML");
    case Xml: return QStringLiteral("XML");
    case Markdown: return QStringLiteral("Markdown");
    case Python: return QStringLiteral("Python");
    case C: return QStringLiteral("C");
    case Cpp: return QStringLiteral("C++");
    case Java: return QStringLiteral("Java");
    case Php: return QStringLiteral("PHP");
    case Shell: return QStringLiteral("Shell");
    case Yaml: return QStringLiteral("YAML");
    case PlainText: break;
    }
    return QStringLiteral("Plain Text");
}

QStringList Highlighter::languageNames() {
    QStringList names;
    for (int i = PlainText; i <= Yaml; ++i) names << languageName(static_cast<Language>(i));
    return names;
}

Highlighter::Language Highlighter::languageFromName(const QString &name) {
    for (int i = PlainText; i <= Yaml; ++i) {
        if (languageName(static_cast<Language>(i)) == name) return static_cast<Language>(i);
    }
    return PlainText;
}

void Highlighter::setLanguage(Language language) {
    if (m_language == language) return;
    m_language = language;
    buildRules();
    rehighlight();
}

/* ------------------------------------------------------------------ */
/* Rules                                                               */
/* ------------------------------------------------------------------ */

void Highlighter::addRule(const QString &pattern, const QTextCharFormat &format, int group) {
    Rule rule;
    rule.pattern = QRegularExpression(pattern);
    rule.format = format;
    rule.group = group;
    m_rules.append(rule);
}

void Highlighter::buildRules() {
    m_rules.clear();

    switch (m_language) {
    case JavaScript:
    case TypeScript:
        addRule(QStringLiteral("\\b[A-Za-z_$][\\w$]*(?=\\s*\\()"), m_fmtFunction);
        break;
    case Cpp:
    case C:
        addRule(QStringLiteral("#\\s*\\w+"), m_fmtMeta);
        addRule(QStringLiteral("\\b[A-Za-z_][\\w]*(?=\\s*\\()"), m_fmtFunction);
        break;
    case Php:
        addRule(QStringLiteral("\\$[A-Za-z_][\\w]*"), m_fmtVariable);
        addRule(QStringLiteral("\\b[A-Za-z_][\\w]*(?=\\s*\\()"), m_fmtFunction);
        break;
    case Java:
        addRule(QStringLiteral("\\b[A-Za-z_][\\w]*(?=\\s*\\()"), m_fmtFunction);
        break;
    case Json:
        addRule(QStringLiteral("\"([^\"\\\\]|\\\\.)*\"(?=\\s*:)"), m_fmtVariable);
        addRule(QStringLiteral("\\b(true|false|null)\\b"), m_fmtNumber);
        addRule(QStringLiteral("-?\\b\\d+(\\.\\d+)?([eE][+-]?\\d+)?\\b"), m_fmtNumber);
        return;
    case Css:
    case Scss:
        addRule(QStringLiteral("@[\\w-]+"), m_fmtKeyword);
        addRule(QStringLiteral("#[0-9a-fA-F]{3,8}\\b"), m_fmtNumber);
        addRule(QStringLiteral("\\b[\\w-]+(?=\\s*:)"), m_fmtAttribute);
        addRule(QStringLiteral("[.#][\\w-]+"), m_fmtVariable);
        addRule(QStringLiteral("!\\s*important"), m_fmtKeyword);
        addRule(QStringLiteral("-?\\b\\d+(\\.\\d+)?(px|em|rem|%|vh|vw|s|ms|deg)?\\b"), m_fmtNumber);
        return;
    case Yaml:
        addRule(QStringLiteral("^\\s*[-\\w]+(?=\\s*:)"), m_fmtAttribute);
        addRule(QStringLiteral("\\b(true|false|null|yes|no)\\b"), m_fmtNumber);
        addRule(QStringLiteral("-?\\b\\d+(\\.\\d+)?\\b"), m_fmtNumber);
        return;
    case Shell:
        addRule(QStringLiteral("\\$\\{?[A-Za-z_][\\w]*\\}?"), m_fmtVariable);
        addRule(QStringLiteral("\\b(if|then|else|elif|fi|for|while|do|done|case|esac|function|in|return|local|export|echo|exit|source)\\b"), m_fmtKeyword);
        return;
    case Python:
        addRule(QStringLiteral("@[\\w.]+"), m_fmtMeta);
        addRule(QStringLiteral("\\b(and|as|assert|async|await|break|class|continue|def|del|elif|else|except|False|finally|for|from|global|if|import|in|is|lambda|None|nonlocal|not|or|pass|raise|return|True|try|while|with|yield)\\b"), m_fmtKeyword);
        addRule(QStringLiteral("\\b[A-Za-z_][\\w]*(?=\\s*\\()"), m_fmtFunction);
        return;
    default:
        break;
    }

    if (m_language == JavaScript || m_language == TypeScript) {
        addRule(QStringLiteral("\\b(await|async|break|case|catch|class|const|continue|debugger|default|delete|do|else|enum|export|extends|finally|for|function|if|implements|import|in|instanceof|interface|let|new|package|private|protected|public|return|static|super|switch|this|throw|try|typeof|var|void|while|with|yield|get|set|of)\\b"), m_fmtKeyword);
        addRule(QStringLiteral("\\b(true|false|null|undefined|NaN|Infinity)\\b"), m_fmtNumber);
    } else if (m_language == C || m_language == Cpp) {
        addRule(QStringLiteral("\\b(alignas|alignof|auto|bool|break|case|catch|char|class|const|constexpr|continue|decltype|default|delete|do|double|else|enum|explicit|export|extern|false|final|float|for|friend|goto|if|inline|int|long|mutable|namespace|new|noexcept|nullptr|operator|override|private|protected|public|register|return|short|signed|sizeof|static|struct|switch|template|this|throw|true|try|typedef|typename|union|unsigned|using|virtual|void|volatile|wchar_t|while)\\b"), m_fmtKeyword);
    } else if (m_language == Java) {
        addRule(QStringLiteral("\\b(abstract|assert|boolean|break|byte|case|catch|char|class|const|continue|default|do|double|else|enum|extends|final|finally|float|for|goto|if|implements|import|instanceof|int|interface|long|native|new|package|private|protected|public|return|short|static|strictfp|super|switch|synchronized|this|throw|throws|transient|try|void|volatile|while|true|false|null|var)\\b"), m_fmtKeyword);
    } else if (m_language == Php) {
        addRule(QStringLiteral("\\b(abstract|and|array|as|break|callable|case|catch|class|clone|const|continue|declare|default|do|echo|else|elseif|empty|enddeclare|endfor|endforeach|endif|endswitch|endwhile|extends|final|finally|fn|for|foreach|function|global|if|implements|include|instanceof|insteadof|interface|isset|list|namespace|new|or|print|private|protected|public|require|return|static|switch|throw|trait|try|unset|use|var|while|xor|yield|true|false|null)\\b"), m_fmtKeyword);
    }

    if (m_language != PlainText && m_language != Markdown && m_language != Html && m_language != Xml) {
        addRule(QStringLiteral("\\b-?0[xX][0-9a-fA-F]+\\b"), m_fmtNumber);
        addRule(QStringLiteral("\\b-?\\d+(\\.\\d+)?([eE][+-]?\\d+)?\\b"), m_fmtNumber);
    }
}

/* ------------------------------------------------------------------ */
/* Highlighting                                                        */
/* ------------------------------------------------------------------ */

void Highlighter::markTaken(int start, int count) {
    m_taken.append(qMakePair(start, count));
}

bool Highlighter::isTaken(int start, int count) const {
    for (const auto &range : m_taken) {
        const int rs = range.first;
        const int re = range.first + range.second;
        if (start < re && start + count > rs) return true;
    }
    return false;
}

void Highlighter::applyRules(const QString &text) {
    for (const Rule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            if (!match.hasMatch()) continue;
            const int start = match.capturedStart(rule.group);
            const int count = match.capturedLength(rule.group);
            if (start < 0 || count <= 0) continue;
            if (isTaken(start, count)) continue;
            setFormat(start, count, rule.format);
            markTaken(start, count);
        }
    }
}

void Highlighter::highlightCStyle(const QString &text) {
    /* block comments /* ... */
    int start = 0;
    if (previousBlockState() == 1) {
        const int end = text.indexOf(QStringLiteral("*/"));
        if (end == -1) {
            setFormat(0, text.length(), m_fmtComment);
            setCurrentBlockState(1);
            markTaken(0, text.length());
            return;
        }
        setFormat(0, end + 2, m_fmtComment);
        markTaken(0, end + 2);
        start = end + 2;
    }

    for (int i = start; i < text.length(); ++i) {
        if (text[i] == QLatin1Char('/') && i + 1 < text.length() && text[i + 1] == QLatin1Char('*')) {
            const int end = text.indexOf(QStringLiteral("*/"), i + 2);
            if (end == -1) {
                setFormat(i, text.length() - i, m_fmtComment);
                markTaken(i, text.length() - i);
                setCurrentBlockState(1);
                break;
            }
            setFormat(i, end + 2 - i, m_fmtComment);
            markTaken(i, end + 2 - i);
            i = end + 1;
        }
    }

    /* line comments */
    QRegularExpression lineComment(QStringLiteral("//[^\\n]*"));
    if (m_language == Shell) lineComment.setPattern(QStringLiteral("#[^\\n]*"));
    if (m_language == Yaml) lineComment.setPattern(QStringLiteral("#[^\\n]*"));
    if (m_language != Python) {
        QRegularExpressionMatchIterator it = lineComment.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            const int start2 = match.capturedStart();
            const int count = match.capturedLength();
            if (isTaken(start2, count)) continue;
            setFormat(start2, count, m_fmtComment);
            markTaken(start2, count);
        }
    }
}

void Highlighter::highlightMarkup(const QString &text) {
    /* <!-- comments --> */
    int from = 0;
    if (previousBlockState() == 2) {
        const int end = text.indexOf(QStringLiteral("-->"));
        if (end == -1) {
            setFormat(0, text.length(), m_fmtComment);
            markTaken(0, text.length());
            setCurrentBlockState(2);
            return;
        }
        setFormat(0, end + 3, m_fmtComment);
        markTaken(0, end + 3);
        from = end + 3;
    }
    for (int i = from; i < text.length() - 3; ++i) {
        if (text.mid(i, 4) == QLatin1String("<!--")) {
            const int end = text.indexOf(QStringLiteral("-->"), i + 4);
            if (end == -1) {
                setFormat(i, text.length() - i, m_fmtComment);
                markTaken(i, text.length() - i);
                setCurrentBlockState(2);
                break;
            }
            setFormat(i, end + 3 - i, m_fmtComment);
            markTaken(i, end + 3 - i);
            i = end;
        }
    }

    /* <tag attr="value"> */
    QRegularExpression tagRE(QStringLiteral("<\\s*/?([A-Za-z][\\w:-]*)"));
    QRegularExpression attrRE(QStringLiteral("([A-Za-z_:][\\w:.-]*)=(\"[^\"]*\"|'[^']*')"));
    QRegularExpressionMatchIterator tags = tagRE.globalMatch(text);
    while (tags.hasNext()) {
        const QRegularExpressionMatch m = tags.next();
        if (isTaken(m.capturedStart(), m.capturedLength())) continue;
        setFormat(m.capturedStart(1), m.capturedLength(1), m_fmtVariable);
        markTaken(m.capturedStart(1), m.capturedLength(1));
    }
    QRegularExpressionMatchIterator attrs = attrRE.globalMatch(text);
    while (attrs.hasNext()) {
        const QRegularExpressionMatch m = attrs.next();
        if (isTaken(m.capturedStart(1), m.capturedLength(1))) continue;
        setFormat(m.capturedStart(1), m.capturedLength(1), m_fmtAttribute);
        markTaken(m.capturedStart(1), m.capturedLength(1));
        setFormat(m.capturedStart(2), m.capturedLength(2), m_fmtString);
        markTaken(m.capturedStart(2), m.capturedLength(2));
    }
}

void Highlighter::highlightPython(const QString &text) {
    /* triple-quoted strings */
    const QString triple = QStringLiteral("\"\"\"");
    const QString tripleSingle = QStringLiteral("'''");
    int pos = 0;
    int state = previousBlockState(); /* 3 = in double, 4 = in single */
    while (pos < text.length()) {
        if (state == 0) {
            const int d = text.indexOf(triple, pos);
            const int s = text.indexOf(tripleSingle, pos);
            int start = -1;
            QString token;
            int newState = 0;
            if (d != -1 && (s == -1 || d < s)) {
                start = d;
                token = triple;
                newState = 3;
            } else if (s != -1) {
                start = s;
                token = tripleSingle;
                newState = 4;
            }
            if (start == -1) break;
            const int end = text.indexOf(token, start + 3);
            if (end == -1) {
                setFormat(start, text.length() - start, m_fmtString);
                markTaken(start, text.length() - start);
                setCurrentBlockState(newState);
                break;
            }
            setFormat(start, end + 3 - start, m_fmtString);
            markTaken(start, end + 3 - start);
            pos = end + 3;
        } else {
            const QString token = state == 3 ? triple : tripleSingle;
            const int end = text.indexOf(token, pos);
            if (end == -1) {
                setFormat(pos, text.length() - pos, m_fmtString);
                markTaken(pos, text.length() - pos);
                setCurrentBlockState(state);
                break;
            }
            setFormat(pos, end + 3 - pos, m_fmtString);
            markTaken(pos, end + 3 - pos);
            pos = end + 3;
            state = 0;
        }
    }

    QRegularExpression comment(QStringLiteral("#[^\\n]*"));
    QRegularExpressionMatchIterator it = comment.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const int start = match.capturedStart();
        const int count = match.capturedLength();
        if (isTaken(start, count)) continue;
        setFormat(start, count, m_fmtComment);
        markTaken(start, count);
    }
}

void Highlighter::highlightBlock(const QString &text) {
    m_taken.clear();
    setCurrentBlockState(0);

    if (text.isEmpty()) return;

    switch (m_language) {
    case PlainText:
        return;
    case Markdown: {
        static const QRegularExpression heading(QStringLiteral("^#{1,6}.*"));
        QRegularExpressionMatch m = heading.match(text);
        if (m.hasMatch() && m.capturedStart() == 0) {
            setFormat(0, text.length(), m_fmtVariable);
            return;
        }
        static const QRegularExpression code(QStringLiteral("`[^`]+`"));
        QRegularExpressionMatchIterator it = code.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), m_fmtString);
        }
        static const QRegularExpression link(QStringLiteral("\\[([^\\]]+)\\]\\(([^)]+)\\)"));
        QRegularExpressionMatchIterator links = link.globalMatch(text);
        while (links.hasNext()) {
            const QRegularExpressionMatch match = links.next();
            setFormat(match.capturedStart(), match.capturedLength(), m_fmtFunction);
        }
        static const QRegularExpression emphasis(QStringLiteral("\\*[^*]+\\*"));
        QRegularExpressionMatchIterator ems = emphasis.globalMatch(text);
        while (ems.hasNext()) {
            const QRegularExpressionMatch match = ems.next();
            QTextCharFormat fmt = m_fmtNumber;
            fmt.setFontItalic(true);
            setFormat(match.capturedStart(), match.capturedLength(), fmt);
        }
        return;
    }
    case Html:
    case Xml:
        highlightMarkup(text);
        return;
    case Python:
        highlightPython(text);
        applyRules(text);
        return;
    case Json:
    case Css:
    case Scss:
    case Yaml:
    case Shell:
        highlightCStyle(text);
        applyRules(text);
        return;
    default:
        break;
    }

    /* JS / TS / C / C++ / Java / PHP */
    /* strings first */
    static const QRegularExpression strings(QStringLiteral("\"([^\"\\\\]|\\\\.)*\"|'([^'\\\\]|\\\\.)*'|`([^`\\\\]|\\\\.)*`"));
    QRegularExpressionMatchIterator sit = strings.globalMatch(text);
    while (sit.hasNext()) {
        const QRegularExpressionMatch match = sit.next();
        const int start = match.capturedStart();
        const int count = match.capturedLength();
        if (isTaken(start, count)) continue;
        setFormat(start, count, m_fmtString);
        markTaken(start, count);
    }

    highlightCStyle(text);
    applyRules(text);
}
