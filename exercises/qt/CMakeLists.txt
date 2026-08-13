cmake_minimum_required(VERSION 3.19)

project(new-pro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =========================
# Qt
# =========================

if(WIN32)
    # Windows 使用 Qt 5.12
    find_package(Qt5 5.12 REQUIRED COMPONENTS Core Widgets)

    set(QT_CORE Qt5::Core)
    set(QT_WIDGETS Qt5::Widgets)

else()
    # macOS 使用 Qt 6
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

    set(QT_CORE Qt6::Core)
    set(QT_WIDGETS Qt6::Widgets)
endif()

# =========================
# Executable
# =========================

add_executable(new-pro
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    logindialog.cpp
    logindialog.h
    logindialog.ui

    pic.qrc
)

# =========================
# Qt Libraries
# =========================

target_link_libraries(new-pro
    PRIVATE
        ${QT_CORE}
        ${QT_WIDGETS}
)

# =========================
# Platform settings
# =========================

if(WIN32)
    set_target_properties(new-pro PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(APPLE)
    set_target_properties(new-pro PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()