cmake_minimum_required(VERSION 3.21)

project(CIDEQt VERSION 1.13.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Pure C++/Qt Widgets application — no QML, no Qt Quick, no WebView.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

set(CIDE_SOURCES
    src/main.cpp
    src/lang.cpp
    src/theme.cpp
    src/icons.cpp
    src/appstate.cpp
    src/headerbar.cpp
    src/filetabs.cpp
    src/codeeditor.cpp
    src/highlighter.cpp
    src/filetree.cpp
    src/quicktools.cpp
    src/welcometab.cpp
    src/settingsdialog.cpp
    src/mainwindow.cpp
)

set(CIDE_HEADERS
    src/lang.h
    src/theme.h
    src/icons.h
    src/appstate.h
    src/headerbar.h
    src/filetabs.h
    src/codeeditor.h
    src/highlighter.h
    src/filetree.h
    src/quicktools.h
    src/welcometab.h
    src/settingsdialog.h
    src/mainwindow.h
)

qt_add_executable(CIDEQt ${CIDE_SOURCES} ${CIDE_HEADERS})

qt_add_resources(CIDEQt "cideres"
    PREFIX "/"
    FILES res/logo.png
)

target_link_libraries(CIDEQt PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# --- offscreen screenshot tool (desktop only, used for UI testing) --------
if(NOT ANDROID)
    set(CIDE_LIB_SOURCES ${CIDE_SOURCES})
    list(REMOVE_ITEM CIDE_LIB_SOURCES src/main.cpp)
    qt_add_executable(CIDEScreenshot EXCLUDE_FROM_ALL
        tools/screenshot_main.cpp
        ${CIDE_LIB_SOURCES}
    )
    target_include_directories(CIDEScreenshot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(CIDEScreenshot PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
    qt_add_resources(CIDEScreenshot "cideres_shot"
        PREFIX "/"
        FILES res/logo.png
    )
endif()

# --- Android packaging -----------------------------------------------------
set_target_properties(CIDEQt PROPERTIES
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
)

if(ANDROID)
    target_link_libraries(CIDEQt PRIVATE Qt6::Widgets)
endif()
