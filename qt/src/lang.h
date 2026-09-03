#pragma once

#include <QString>
#include <QStringList>

/*
 * Interface strings taken from src/lang/en-us.json and src/lang/ru-ru.json
 * at commit 3673f5a (only English and Russian are shipped in CIDE).
 */
class Lang {
public:
    enum Language { English = 0, Russian = 1 };

    static void set(Language language);
    static Language current();
    static QString currentCode(); /* "en-us" or "ru-ru" */
    static bool setFromCode(const QString &code);
    static QStringList languageNames();

    /* Returns the translated string for the given key. */
    static QString s(const QString &key);

private:
    struct Entry {
        const char *key;
        const char *en;
        const char *ru;
    };
    static const Entry kStrings[];
    static Language m_current;
};
