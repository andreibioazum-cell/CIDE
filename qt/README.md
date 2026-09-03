# CIDE Qt (C++ / Qt Widgets)

Оболочка CIDE, полностью переписанная на **C++ / Qt Widgets** — без QML,
без Qt Quick и без WebView. Интерфейс повторяет веб-интерфейс CIDE из
коммита `3673f5a`:

- тема «Dark» из `src/theme/preInstalled.js` (primary `#232729`,
  active `#4285f4`), редактор в стиле CodeMirror one_dark;
- заголовок (tile header) 45px: меню-бургер, название файла, кнопка
  меню файла, запуск, «⋮»;
- список открытых файлов (30px, вкладки 120px, маркер «•» у
  несохранённых, активная вкладка с синей полосой сверху);
- сайдбар с панелью приложений (Files, Search, Extensions,
  Notifications) и деревом файлов открытых хранилищ;
- быстрые инструменты (quick tools) внизу — те же кнопки, что в
  `src/components/quickTools/items.js`, включая режим поиска/замены;
- вкладка «Get Started» и настройки (язык EN/RU, шрифт, табы и т.д.);
- все строки интерфейса взяты из `src/lang/en-us.json` / `ru-ru.json`.

Строки и настройки по умолчанию (fontSize 12, tabSize 2, softTab,
one_dark и т.д.) соответствуют `src/lib/settings.js` того коммита.

## Сборка (десктоп)

```sh
cmake -S qt -B qt/build-desktop -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build qt/build-desktop
./qt/build-desktop/CIDEQt
```

Нужен Qt 6 (модули Core, Gui, Widgets) и CMake ≥ 3.21.

## Сборка (Android)

Установить Qt 6 с Android kit, Android SDK и NDK r26d
(`26.1.10909125`) и выполнить:

```sh
"$QT_ROOT_DIR/bin/qt-cmake" -S qt -B qt/build -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DQT_HOST_PATH="$QT_HOST_PATH" \
  -DANDROID_SDK_ROOT="$ANDROID_HOME" \
  -DANDROID_NDK_ROOT="$ANDROID_HOME/ndk/26.1.10909125"
cmake --build qt/build --target CIDEQt_make_apk
```

`QT_HOST_PATH` должен указывать на host-сборку Qt для Linux
(`.../gcc_64`). APK появится в `qt/build/android-build/CIDEQt.apk`.

CI собирает и десктоп-версию (проверка компиляции), и APK — см.
`.github/workflows/ci.yml`.
