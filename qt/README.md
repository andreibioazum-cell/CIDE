# CIDE Qt Android

Новая оболочка приложения написана на Qt 6/QML и C++. Единственная веб-часть — сайт в `QtWebView`; интерфейс сайта не переносится в Qt.

## Сборка

Установить Qt 6 с Android kit (например, `aquila`/`aqtinstall`), Android SDK и NDK r26d (`26.1.10909125`) и выполнить:

```sh
"$QT_ROOT_DIR/bin/qt-cmake" -S qt -B qt/build -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DQT_HOST_PATH="$QT_HOST_PATH" \
  -DANDROID_SDK_ROOT="$ANDROID_HOME" \
  -DANDROID_NDK_ROOT="$ANDROID_HOME/ndk/26.1.10909125"
cmake --build qt/build --target CIDEQt_make_apk
```

`qt-cmake` подставляет Qt-специфичный toolchain-файл (`lib/cmake/Qt6/qt.toolchain.cmake`), который цепляет Android NDK. `QT_HOST_PATH` должен указывать на host-сборку Qt для Linux (`.../gcc_64`), а не на Android kit. APK появится в `qt/build/android-build/CIDEQt.apk`.

URL сайта меняется в `qt/qml/Main.qml` через `siteUrl`.
