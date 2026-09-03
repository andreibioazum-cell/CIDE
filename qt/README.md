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
  Notifications) и деревом файлов открытых хранилищ — как в вебе,
  это **выезжающий drawer поверх контента** (`#sidebar {
  position: fixed; width: 70vw; max-width: 350px }`), а не
  пристыкованная панель: закрывается кнопкой-бургером или кликом
  по области справа;
- быстрые инструменты (quick tools) внизу — те же кнопки, что в
  `src/components/quickTools/items.js`, включая режим поиска/замены;
  ряды кнопок прокручиваются по горизонтали
  (`.button-container { overflow-x: auto }`), а не растягивают окно;
- на вкладке «Get Started» быстрые инструменты скрыты
  (`hideQuickTools: true` у welcome-страницы в вебе);
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

## Скриншоты без дисплея (offscreen)

Для проверки интерфейса без X11/Wayland служит утилита
`CIDEScreenshot` (цель `CIDEScreenshot`, в `all` не входит):

```sh
cmake --build qt/build-desktop --target CIDEScreenshot
QT_QPA_PLATFORM=offscreen \
QT_QPA_FONTDIR=/usr/share/fonts/truetype/dejavu \
./qt/build-desktop/CIDEScreenshot <каталог-для-png> [en-us|ru-ru]
```

`QT_QPA_FONTDIR` должен указывать на каталог с файлами `.ttf`
напрямую (без fontconfig basic-база шрифтов Qt не обходит
подкаталоги) — иначе весь текст отрисуется «квадратиками».
Утилита сохраняет 4 снимка: `*-phone` (420×840, редактор),
`*-wide` (1000×800, редактор), `*-sidebar` (drawer открыт) и
`*-welcome` (вкладка «Get Started», локализованные строки).

CI в репозитории сейчас собирает веб-приложение; Qt-сборки
выполняются локально по командам выше.
