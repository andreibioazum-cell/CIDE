# CIDE Qt Android

Новая оболочка приложения написана на Qt 6/QML и C++. Единственная веб-часть — сайт в `QtWebView`; интерфейс сайта не переносится в Qt.

## Сборка

Установить Qt 6 с Android kit и выполнить:

```sh
cmake -S qt -B qt/build -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26
cmake --build qt/build --target CIDEQt
```

URL сайта меняется в `qt/qml/Main.qml` через `siteUrl`.
