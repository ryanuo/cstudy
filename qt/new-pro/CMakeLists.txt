cmake_minimum_required(VERSION 3.21)

project(new-pro VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Widgets
    Sql
)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

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

    registerdialog.cpp
    registerdialog.h
    registerdialog.ui

    dbhelper.cpp
    dbhelper.h

    pic.qrc
)

target_link_libraries(new-pro
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
)

include(GNUInstallDirs)

install(TARGETS new-pro
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET new-pro
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})