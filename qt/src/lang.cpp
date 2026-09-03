#include "lang.h"

#include <QHash>

Lang::Language Lang::m_current = Lang::English;

const Lang::Entry Lang::kStrings[] = {
    /* main menu / file menu (src/views/menu.hbs, file-menu.hbs) */
    { "new file", "New file", "Новый файл" },
    { "save", "Save", "Сохранить" },
    { "save as", "Save as", "Сохранить как" },
    { "save file", "Save file", "Сохранить файл" },
    { "save all", "Save all", "Сохранить всё" },
    { "files", "Files", "Файлы" },
    { "close file", "Close file", "Закрыть файл" },
    { "close", "Close", "Закрыть" },
    { "close all", "Close all", "Закрыть всё" },
    { "open recent", "Open recent", "Открыть недавние" },
    { "find file", "Find file", "Поиск" },
    { "console", "Console", "Консоль" },
    { "terminal", "Terminal", "Терминал" },
    { "running processes", "Running processes", "Running processes" },
    { "settings", "Settings", "Настройки" },
    { "help", "Help", "Помощь" },
    { "exit", "Exit", "Выйти" },
    { "file properties", "File properties", "Свойство файла" },
    { "rename", "Rename", "Переименовать" },
    { "syntax highlighting", "Syntax highlighting", "Подсветка синтаксиса" },
    { "encoding", "Text encoding", "Кодировка" },
    { "new line mode", "New line mode", "Перенос строки" },
    { "read only", "Read only", "Только для чтения" },
    { "format", "Format", "Форматировать" },
    { "share", "Share", "Поделиться" },
    { "open with", "Open with", "Открыть в..." },
    { "edit with", "Edit with", "Редактировать в..." },
    { "add to home screen", "Add to home screen", "Add to home screen" },
    { "pin tab", "Pin tab", "Pin tab" },
    { "unpin tab", "Unpin tab", "Unpin tab" },
    { "close tabs to right", "Close Right", "Close Right" },
    { "close tabs to left", "Close Left", "Close Left" },
    { "close other tabs", "Close Others", "Close Others" },
    { "search", "Search", "Поиск" },
    { "goto", "Goto line", "Перейти к строке" },
    { "insert color", "Insert color", "Вставить цвет" },
    { "cut", "Cut", "Вырезать" },
    { "paste", "Paste", "Вставить" },
    { "copy", "Copy", "Копировать" },
    { "select all", "Select all", "Выбрать всё" },
    { "replace", "Replace", "Заменить" },
    { "command palette", "Open Command Palette", "Open Command Palette" },

    /* dialogs */
    { "cancel", "Cancel", "Отмена" },
    { "ok", "ok", "OК" },
    { "open file", "Open file", "Открыть файл" },
    { "open folder", "Open folder", "Открыть папку" },
    { "select folder", "Select folder", "Выбрать папку" },
    { "new folder", "New Folder", "Новая папка" },
    { "folder name", "Folder name", "Имя папки" },
    { "type filename", "Type filename", "Введите имя файла" },
    { "delete", "Delete", "Удалить" },
    { "file already exists", "File already exists", "Файл уже существует" },
    { "unable to open file", "Sorry, unable to open file", "Невозможно открыть файл" },
    { "unable to open folder", "Sorry, unable to open folder", "Невозможно открыть папку" },
    { "unable to save file", "Sorry, unable to save file", "Невозможно сохранить файл" },
    { "unable to rename", "Sorry, unable to rename", "Невозможно переименовать" },
    { "unable to delete file", "unable to delete file", "Невозможно удалить файл" },
    { "create folder error", "Sorry, unable create new folder", "Не удалось создать папку" },
    { "an error occurred", "An error occurred", "Произошла ошибка" },
    { "file saved", "file saved", "Файл сохранен" },
    { "file renamed", "file renamed", "Файл переименован" },
    { "file deleted", "File deleted", "Файл удалён" },
    { "unsaved file", "This file is not saved, close anyway?", "Файл не сохранён, закрыть?" },
    { "unsaved files close app", "There are unsaved files. Close application?", "Имеются несохранённые файлы. Всё равно выйти?" },
    { "delete entry", "Confirm deletion: '{name}'. This action cannot be undone. Proceed?", "Удалить '{name}'? Действие отменить невозможно." },
    { "enter line number", "Enter line number", "Введите номер строки" },
    { "enter new name", "Enter new name", "Enter new name" },
    { "new name", "New name", "New name" },
    { "add a storage", "Add a storage", "Добавить хранилище" },
    { "empty folder message", "Empty Folder", "Пустая папка" },
    { "loading", "Loading", "Загрузка" },

    /* sidebar apps */
    { "open files and folders", "Open files and folders", "Открытые файлы и папки" },
    { "search in files", "Search in files", "Найти в файлах" },
    { "plugins", "Plugins", "Плагины" },
    { "no plugins found", "No plugins found", "Плагины не найдены" },
    { "explore", "Explore", "Поиск плагинов" },
    { "notifications", "Notifications", "Уведомления" },
    { "no_unread_notifications", "No unread notifications", "Нет непрочитанных уведомлений" },
    { "recent", "Recent Files", "Recent Files" },
    { "problems", "Problems", "Проблемы" },

    /* welcome page (src/pages/welcome/welcome.js) */
    { "about", "About", "О приложении" },
    { "website", "Website", "Website" },

    /* settings (src/settings/*) */
    { "app settings", "App settings", "Настройки приложения" },
    { "editor settings", "Editor settings", "Настройки редактора" },
    { "terminal settings", "Terminal Settings", "Настройки терминала" },
    { "settings saved", "Settings saved", "Настройки сохранены" },
    { "language", "Language", "Язык" },
    { "change language", "Change language", "Изменить язык" },
    { "language changed", "language has been changed successfully", "Язык успешно изменен" },
    { "theme", "Theme", "Тема" },
    { "editor font", "Editor font", "Шрифт" },
    { "font size", "Font size", "Размер шрифта" },
    { "tab size", "Tab size", "Количество пробелов в табе" },
    { "soft tab", "Soft tab", "Использовать пробелы вместо Tab" },
    { "text wrap", "Text wrap / Word wrap", "Перенос строк" },
    { "line height", "Line height", "Высота строки" },
    { "show line numbers", "Show line numbers", "Показывать нумерацию строк" },
    { "relative line numbers", "Relative line numbers", "Относительный номер строки" },
    { "quick tools", "Quick tools", "Быстрые инструменты" },
    { "info-quickTools", "Show or hide quick tools.", "Show or hide quick tools." },
    { "fullscreen", "Fullscreen", "Полноэкранный режим" },
    { "autosave", "Autosave", "Автосохранение" },
    { "format on save", "Format on save", "Форматирование при сохранении" },
    { "remember opened files", "Remember opened files", "Запоминать открытые файлы" },
    { "remember opened folders", "Remember opened folders", "Запоминать открытые папки" },
    { "settings-info-app-language", "Choose the app language and translated labels.", "Choose the app language and translated labels." },
    { "settings-info-editor-line-numbers", "Show line numbers in the gutter.", "Show line numbers in the gutter." },
    { "open source", "Open Source", "Открытый исходный код" },
    { "path", "Path", "Путь" },
    { "line count", "Line count", "Количество строк" },
    { "size", "Size", "Размер" },
    { "type", "Type", "Тип" },
    { "directory", "Directory", "Папка" },
    { "file", "File", "Файл" },

    /* quick tools tooltips (src/components/quickTools/items.js) */
    { "quicktools:ctrl-key", "Control/Command key", "Control/Command клавиша" },
    { "quicktools:shift-key", "Shift key", "Клав. Shift" },
    { "quicktools:alt-key", "Alt key", "Клав. Alt" },
    { "quicktools:meta-key", "Windows/Meta key", "Клав. Windows/Meta" },
    { "quicktools:esc-key", "Escape key", "Клав. Escape" },
    { "quicktools:tab-key", "Tab key", "Клав. Tab" },
    { "quicktools:undo", "Undo", "Отмена" },
    { "quicktools:redo", "Redo", "Повтор" },
    { "quicktools:search", "Search in file", "Поиск в файле" },
    { "quicktools:save", "Save file", "Сохранить файл" },
    { "quicktools:search-prev", "Previous Match", "Previous Match" },
    { "quicktools:search-next", "Next Match", "Next Match" },
    { "quicktools:search-settings", "Search Settings", "Search Settings" },
    { "quicktools:search-replace", "Replace", "Replace" },
    { "quicktools:search-replace-all", "Replace All", "Replace All" },
    { "quicktools:moveline-up", "Move line up", "Перенос на линию выше" },
    { "quicktools:moveline-down", "Move line down", "Перенос на линию ниже" },
    { "quicktools:copyline-up", "Copy line up", "Копировать линию выше" },
    { "quicktools:copyline-down", "Copy line down", "Копировать линию ниже" },
    { "quicktools:command-palette", "Command palette", "Палитра команд" },
    { "quicktools:semicolon", "Insert semicolon", "Вставить точку с запятой" },
    { "quicktools:quotation", "Insert quotation", "Вставить цитату" },
    { "quicktools:and", "Insert and symbol", "Вставка и символ" },
    { "quicktools:bar", "Insert bar symbol", "Вставка символа полосы" },
    { "quicktools:equal", "Insert equal symbol", "Вставка эквивалентного символа" },
    { "quicktools:slash", "Insert slash symbol", "Вставка слэш-символа" },
    { "quicktools:exclamation", "Insert exclamation", "Вставить восклицательный знак" },
    { "quicktools:tilde", "Insert tilde symbol", "Вставить тильду" },
    { "quicktools:backtick", "Insert backtick", "Вставить обратный апостроф" },
    { "quicktools:hash", "Insert Hash symbol", "Вставить символ #" },
    { "quicktools:dollar", "Insert dollar symbol", "Вставить символ доллара" },
    { "quicktools:modulo", "Insert modulo/percent symbol", "Вставить символ процента" },
    { "quicktools:caret", "Insert caret symbol", "Вставить символ каретки" },
    { "quicktools:hyphen", "Insert hyphen symbol", "Вставить символ дефиса" },
    { "quicktools:curlybracket", "Insert curly bracket", "Вставить фигурную скобку" },
    { "quicktools:squarebracket", "Insert square bracket", "Вставить квадратную скобку" },
    { "quicktools:parentheses", "Insert parentheses", "Вставить круглые скобки" },
    { "quicktools:anglebracket", "Insert angle bracket", "Вставить угловую скобку" },
    { "quicktools:up-arrow-key", "Up arrow key", "Клав. стрелка вверх " },
    { "quicktools:down-arrow-key", "Down arrow key", "Клав. стрелка вниз " },
    { "quicktools:left-arrow-key", "Left arrow key", "Клав. стрелка влево" },
    { "quicktools:right-arrow-key", "Right arrow key", "Клав. стрелка вправо" },
    { "quicktools:home-key", "Home Key", "Клав. Home" },
    { "quicktools:end-key", "End Key", "Клав. End" },
    { "quicktools:pageup-key", "PageUp Key", "Клав. PageUp" },
    { "quicktools:pagedown-key", "PageDown Key", "Клав. PageDown" },
    { "quicktools:delete-key", "Delete Key", "Клав. Delete" },
};

void Lang::set(Language language) { m_current = language; }

Lang::Language Lang::current() { return m_current; }

QString Lang::currentCode() {
    return m_current == Russian ? QStringLiteral("ru-ru") : QStringLiteral("en-us");
}

bool Lang::setFromCode(const QString &code) {
    if (code == QLatin1String("ru-ru")) {
        m_current = Russian;
        return true;
    }
    if (code == QLatin1String("en-us")) {
        m_current = English;
        return true;
    }
    return false;
}

QStringList Lang::languageNames() {
    return { QStringLiteral("English"), QStringLiteral("Русский") };
}

QString Lang::s(const QString &key) {
    static QHash<QString, const Entry *> cache;
    if (cache.isEmpty()) {
        const int count = sizeof(kStrings) / sizeof(kStrings[0]);
        for (int i = 0; i < count; ++i) cache.insert(QString::fromLatin1(kStrings[i].key), &kStrings[i]);
    }
    const Entry *entry = cache.value(key, nullptr);
    if (!entry) return key;
    return QString::fromUtf8(m_current == Russian ? entry->ru : entry->en);
}
