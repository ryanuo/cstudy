cmake_minimum_required(VERSION 3.16)

project(qt_practice VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Apple Silicon macOS Qt 6.11.1
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "$ENV{HOME}/Qt/6.11.1/macos")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# -- 唯一 CMakeLists：动态扫描编号练习目录，为每个目录生成一个可执行目标
# 目录约定: NN_name/  (NN 为两位数字编号)
#   目标名/可执行名 = 目录名下划线后部分 (01_hello -> hello)
#   产物输出到 lib/NN_name/hello
file(GLOB EXERCISE_DIRS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
     CONFIGURE_DEPENDS "[0-9][0-9]_*")

foreach(EXDIR ${EXERCISE_DIRS})
    if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${EXDIR}")
        continue()
    endif()

    # 目标名取目录名下划线后部分, 如 01_hello -> hello
    string(REGEX REPLACE "^[0-9][0-9]_" "" EXE_NAME "${EXDIR}")

    # 收集该目录下所有源文件(含 .h 供 AUTOMOC 处理 Q_OBJECT)
    file(GLOB EX_SOURCES CONFIGURE_DEPENDS
         "${EXDIR}/*.cpp" "${EXDIR}/*.h")

    # 跳过空目录(尚无源文件的练习不会导致构建失败)
    if(NOT EX_SOURCES)
        message(STATUS "跳过空练习目录: ${EXDIR} (无 .cpp/.h 源文件)")
        continue()
    endif()

    add_executable(${EXE_NAME} ${EX_SOURCES})

    # 产物输出到 lib/NN_name/
    set_target_properties(${EXE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${EXDIR}"
    )

    target_link_libraries(${EXE_NAME} PRIVATE Qt6::Widgets)
endforeach()
