cmake_minimum_required(VERSION 3.16)
project(extart VERSION 0.4 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(WEBKIT REQUIRED webkitgtk-6.0)
find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

set(GRESOURCE_XML ${CMAKE_CURRENT_SOURCE_DIR}/resources/extart.gresource.xml)
set(GRESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/extart-resources.c)

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
    src/profile.cpp
    src/config.cpp
    src/history.cpp
    src/download_manager.cpp
    src/bookmarks.cpp
    src/webkit_memory.cpp
    src/history_panel.cpp
    src/bookmarks_panel.cpp
    src/downloads_panel.cpp
    src/settings_window.cpp
    src/session_manager.cpp
    ${GRESOURCE_C}
)

target_include_directories(extart PRIVATE
    ${GTK4_INCLUDE_DIRS}
    ${WEBKIT_INCLUDE_DIRS}
    src/
)

target_link_libraries(extart PRIVATE
    ${GTK4_LIBRARIES}
    ${WEBKIT_LIBRARIES}
)

target_compile_options(extart PRIVATE
    ${GTK4_CFLAGS_OTHER}
    ${WEBKIT_CFLAGS_OTHER}
)
