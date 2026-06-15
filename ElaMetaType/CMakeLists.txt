cmake_minimum_required(VERSION 3.12)

project(ElaMetaType VERSION 2.0.0 LANGUAGES CXX)

add_definitions(-DELAMETATYPE_LIBRARY)
option(ELAMETATYPE_BUILD_STATIC_LIB "Build static library." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FILE(GLOB ORIGIN CONFIGURE_DEPENDS *.h *.cpp)
FILE(GLOB private CONFIGURE_DEPENDS private/*.h private/*.cpp)
FILE(GLOB pugixml CONFIGURE_DEPENDS pugixml/*.hpp pugixml/*.cpp)
FILE(GLOB Development CONFIGURE_DEPENDS Development/*.h Development/*.cpp)

source_group(private FILES ${private})
source_group(pugixml FILES ${pugixml})

set(PROJECT_SOURCES
    ${ORIGIN}
    ${private}
    ${Development}
    ${pugixml}
)

if (ELAMETATYPE_BUILD_STATIC_LIB)
    set(LIB_TYPE "STATIC")
else ()
    set(LIB_TYPE "SHARED")
endif ()

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${PROJECT_NAME} ${LIB_TYPE}
        ${PROJECT_SOURCES}
    )
else ()
    add_library(${PROJECT_NAME} ${LIB_TYPE}
        ${PROJECT_SOURCES}
    )
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME MetaType)
add_library(Ela::MetaType ALIAS ${PROJECT_NAME})

FILE(GLOB EXPORT_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
FILE(GLOB PUGIXML_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/*.h)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pugixml>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Development>
    $<INSTALL_INTERFACE:${PROJECT_NAME}/include>
    $<INSTALL_INTERFACE:${PROJECT_NAME}/include/pugixml>
)

if (MINGW)
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
endif ()
if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE
    lz4_static
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION ${PROJECT_NAME}/lib
    LIBRARY DESTINATION ${PROJECT_NAME}/lib
    RUNTIME DESTINATION ${PROJECT_NAME}/bin
)

install(FILES ${EXPORT_HEADERS} DESTINATION ${PROJECT_NAME}/include)
install(FILES ${PUGIXML_HEADERS} DESTINATION ${PROJECT_NAME}/include/pugixml)

set(INCLUDE_DIRS include)
set(LIBRARIES ${PROJECT_NAME})
set(LIB_DIR lib)

install(
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE Ela::
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION ${PROJECT_NAME}/lib/cmake
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION 2.0.0
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake
    PATH_VARS INCLUDE_DIRS LIBRARIES LIB_DIR
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
)

install(
    FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake DESTINATION ${PROJECT_NAME}/lib/cmake
)
