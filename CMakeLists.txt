cmake_minimum_required(VERSION 3.12)

project(ElaFramework VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

#在这里配置QT路径 例如 D:/Qt/6.6.2/msvc2019_64 D:/Qt/5.15.2/msvc2019_64 /home/liniyous/Qt/5.15.2/gcc_64
SET(QT_SDK_DIR D:/Qt/5.15.2/msvc2019_64 CACHE PATH "QT SDK DIR" FORCE)
message("在CMAKE中配置 QT_SDK_DIR 选项指定QT路径; 当前路径: ${QT_SDK_DIR}")

list(APPEND CMAKE_PREFIX_PATH ${QT_SDK_DIR})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/Install/${QT_VERSION} CACHE PATH "Installation path" FORCE)

if (NOT WIN32)
    add_link_options(-Wl,--disable-new-dtags)
    set(CMAKE_SKIP_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${QT_SDK_DIR}/lib:${CMAKE_INSTALL_PREFIX}/ElaMetaType/lib")
endif ()

add_subdirectory(3RD/lz4-1.10.0)
add_subdirectory(ElaMetaType)
add_subdirectory(ElaMetaTypeExample)
include(GNUInstallDirs)
