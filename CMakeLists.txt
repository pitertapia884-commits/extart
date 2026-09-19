cmake_minimum_required(VERSION 3.16)
project(extart VERSION 0.4 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

option(EXTART_ENABLE_GECKO "Build the native Gecko embedding backend" ON)

if(EXTART_ENABLE_GECKO)
    set(EXTART_GECKO_ROOT "" CACHE PATH "Root of the EXTART Gecko build/source tree")
    set(EXTART_GECKO_REVISION "b16f852ba66d190cacd42d0d43ac4d38bd07c4bd"
        CACHE STRING "Mozilla source revision expected by EXTART Gecko embedding work")
    if(NOT EXTART_GECKO_ROOT)
        message(FATAL_ERROR "EXTART_ENABLE_GECKO requires -DEXTART_GECKO_ROOT=/path/to/gecko")
    endif()

    if(NOT IS_DIRECTORY "${EXTART_GECKO_ROOT}")
        message(FATAL_ERROR "EXTART_GECKO_ROOT does not exist: ${EXTART_GECKO_ROOT}")
    endif()

    set(EXTART_GECKO_DIST "${EXTART_GECKO_ROOT}/obj-extart/dist")
    set(EXTART_GECKO_INCLUDE "${EXTART_GECKO_DIST}/include")
    set(EXTART_GECKO_LIB "${EXTART_GECKO_DIST}/bin/libxul.so")

    if(NOT IS_DIRECTORY "${EXTART_GECKO_INCLUDE}")
        message(FATAL_ERROR "Gecko generated headers are missing: ${EXTART_GECKO_INCLUDE}")
    endif()

    if(NOT IS_DIRECTORY "${EXTART_GECKO_DIST}/bin")
        message(FATAL_ERROR "Gecko runtime directory is missing: ${EXTART_GECKO_DIST}/bin")
    endif()

    if(NOT EXISTS "${EXTART_GECKO_LIB}")
        message(FATAL_ERROR "Gecko libxul is missing: ${EXTART_GECKO_LIB}")
    endif()

    message(STATUS "EXTART Gecko source: ${EXTART_GECKO_ROOT}")
    message(STATUS "EXTART Gecko revision: ${EXTART_GECKO_REVISION}")
endif()

# Gecko migration boundary. Gecko itself is built separately from Mozilla's
# source tree; EXTART keeps its GTK4 UI independent from Firefox's desktop UI.
# Native Gecko is the only browser engine. The GTK4 UI remains engine-neutral.

set(GRESOURCE_XML ${CMAKE_CURRENT_SOURCE_DIR}/resources/extart.gresource.xml)
set(GRESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/extart-resources.c)

add_custom_command(
    OUTPUT ${GRESOURCE_C}
    COMMAND ${GLIB_COMPILE_RESOURCES}
        --target=${GRESOURCE_C}
        --generate-source
        --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${GRESOURCE_XML}
    DEPENDS ${GRESOURCE_XML}
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/home.html
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/style.css
    VERBATIM
)

add_executable(extart
    src/main.cpp
    src/extart_application.cpp
    src/browser_window.cpp
    src/tab.cpp
    src/browser_engine.hpp
    src/profile.cpp
    src/config.cpp
    src/history.cpp
    src/download_manager.cpp
    src/bookmarks.cpp
    src/history_panel.cpp
    src/bookmarks_panel.cpp
    src/downloads_panel.cpp
    src/settings_window.cpp
    src/session_manager.cpp
    src/gecko_backend.cpp
    $<$<BOOL:${EXTART_ENABLE_GECKO}>:src/gecko_runtime.hpp>
    $<$<BOOL:${EXTART_ENABLE_GECKO}>:src/gecko_runtime.cpp>
    $<$<BOOL:${EXTART_ENABLE_GECKO}>:src/gecko_engine.hpp>
    $<$<BOOL:${EXTART_ENABLE_GECKO}>:src/gecko_engine.cpp>
    ${GRESOURCE_C}
)

target_include_directories(extart PRIVATE
    ${GTK4_INCLUDE_DIRS}
    src/
)

if(EXTART_ENABLE_GECKO)
    target_include_directories(extart PRIVATE
        ${EXTART_GECKO_INCLUDE}
        ${EXTART_GECKO_INCLUDE}/nspr
        ${EXTART_GECKO_ROOT}
        ${EXTART_GECKO_ROOT}/widget
        ${EXTART_GECKO_ROOT}/layout/base
        ${EXTART_GECKO_ROOT}/layout/painting
        ${EXTART_GECKO_ROOT}/gfx/thebes
        ${EXTART_GECKO_ROOT}/gfx/2d
        ${EXTART_GECKO_ROOT}/docshell/base
    )
    target_compile_definitions(extart PRIVATE
        EXTART_ENABLE_GECKO=1
        EXTART_GECKO_REVISION="${EXTART_GECKO_REVISION}")
endif()

target_link_libraries(extart PRIVATE
    ${GTK4_LIBRARIES}
    $<$<BOOL:${EXTART_ENABLE_GECKO}>:${EXTART_GECKO_LIB}>
)

if(EXTART_ENABLE_GECKO)
    set_target_properties(extart PROPERTIES
        BUILD_RPATH "${EXTART_GECKO_DIST}/bin"
        INSTALL_RPATH "${EXTART_GECKO_DIST}/bin")
endif()

target_compile_options(extart PRIVATE
    ${GTK4_CFLAGS_OTHER}
)
