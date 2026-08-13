cmake_minimum_required(VERSION 3.19)

project(new-pro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(new-pro
    WIN32
    MACOSX_BUNDLE

    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    logindialog.cpp
    logindialog.h
    logindialog.ui

    pic.qrc
)

target_link_libraries(new-pro
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)