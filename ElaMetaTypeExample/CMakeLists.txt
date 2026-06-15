cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0087 NEW)
project(ElaMetaTypeExample VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Xml)

FILE(GLOB ORIGIN *.cpp *.h)

set(PROJECT_SOURCES
    ${ORIGIN}
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    #遍历所有资源文件
    file(GLOB_RECURSE RES_PATHS *.png *.jpg *.svg *.ico *.ttf *.webp *.js)
    foreach (filepath ${RES_PATHS})
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
        list(APPEND resource_files ${filename})
    endforeach (filepath)

    qt_add_resources(${PROJECT_NAME} "ElaMetaTypeExample"
        RESOURCES PREFIX "/"
        FILES
        ${resource_files}
    )
else ()
    qt5_add_big_resources(PROJECT_SOURCES
        ElaMetaTypeExample.qrc
    )
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
    Ela::MetaType
)

if (${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME})
endif ()
set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif ()

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
)

install(TARGETS ElaMetaType
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
)

find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_SDK_DIR}/bin")
if (WIN32)
    install(CODE "
    if(EXISTS \"${WINDEPLOYQT_EXECUTABLE}\" AND NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/platforms\")
        execute_process(
            COMMAND \"${CMAKE_COMMAND}\" -E env PATH=\"${QT_SDK_DIR}/bin\"
                \"${WINDEPLOYQT_EXECUTABLE}\"
                \"$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>\"
                --dir \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}\"
                --no-compiler-runtime
                --no-system-d3d-compiler
                --no-opengl-sw
                --verbose 1
        )
    endif()"
    )
endif ()

