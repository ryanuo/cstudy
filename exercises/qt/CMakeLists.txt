cmake_minimum_required(VERSION 3.16)

project(qt_practice VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 6.11.1
set(CMAKE_PREFIX_PATH
    "${CMAKE_PREFIX_PATH}"
    "$ENV{HOME}/Qt/6.11.1/macos"
)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ============================================================
# 动态扫描练习目录
# ============================================================

file(GLOB EXERCISE_DIRS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    CONFIGURE_DEPENDS
    "[0-9][0-9]_*"
)

foreach(EXDIR ${EXERCISE_DIRS})

    if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${EXDIR}")
        continue()
    endif()

    # 01_hello -> hello
    string(
        REGEX REPLACE
        "^[0-9][0-9]_"
        ""
        EXE_NAME
        "${EXDIR}"
    )

    # 收集源文件
    file(GLOB EX_SOURCES
        CONFIGURE_DEPENDS
        "${EXDIR}/*.cpp"
        "${EXDIR}/*.h"
        "${EXDIR}/*.hpp"
        "${EXDIR}/*.qrc"
    )

    if(NOT EX_SOURCES)
        message(
            STATUS
            "跳过空练习目录: ${EXDIR}"
        )
        continue()
    endif()

    # ========================================================
    # 创建可执行程序
    # ========================================================

    add_executable(
        ${EXE_NAME}
        MACOSX_BUNDLE
        ${EX_SOURCES}
    )

    # ========================================================
    # Bundle 信息
    # ========================================================

    set_target_properties(
        ${EXE_NAME}
        PROPERTIES

        MACOSX_BUNDLE_GUI_IDENTIFIER
            "cc.ryanuo.qt.${EXE_NAME}"

        MACOSX_BUNDLE_BUNDLE_NAME
            "${EXE_NAME}"

        MACOSX_BUNDLE_BUNDLE_VERSION
            "${PROJECT_VERSION}"

        MACOSX_BUNDLE_SHORT_VERSION_STRING
            "${PROJECT_VERSION}"

        RUNTIME_OUTPUT_DIRECTORY
            "${CMAKE_SOURCE_DIR}/lib/${EXDIR}"
    )

    # ========================================================
    # Qt
    # ========================================================

    target_link_libraries(
        ${EXE_NAME}
        PRIVATE
        Qt6::Widgets
    )

endforeach()