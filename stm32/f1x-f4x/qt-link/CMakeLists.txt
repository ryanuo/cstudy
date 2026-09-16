cmake_minimum_required(VERSION 3.24)

project(qt_link VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SerialPort 必须用带 QtSerialPort 的 Qt（本机 Homebrew Qt 6.11.2 有，~/Qt/6.11.1 没装 SerialPort）
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml QuickControls2 Svg Network SerialPort)
qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(third_party/qtshadcn)

qt_add_executable(appqtlink
    main.cpp
    SerialLink.h
    SerialLink.cpp
)

qt_add_qml_module(appqtlink
    URI QtLink
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(appqtlink
    PRIVATE
        QtShadcn
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        Qt6::QuickControls2
        Qt6::SerialPort
)

set_target_properties(appqtlink PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
