cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    juntos
    VERSION 0.1.0
    DESCRIPTION "A Peer-to-Peer Game Networking Library"
    HOMEPAGE_URL "https://github.com/nmstory/juntos"
    LANGUAGES CXX
)

# ---- Declare library ----

add_library(
    juntos_lib OBJECT
    src/lib.cpp
)

target_include_directories(
    juntos_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(juntos_lib PUBLIC cxx_std_17)

# ---- Declare executable ----

add_executable(juntos_exe src/main.cpp)
add_executable(juntos::exe ALIAS juntos_exe)

set_property(TARGET juntos_exe PROPERTY OUTPUT_NAME juntos)

target_compile_features(juntos_exe PRIVATE cxx_std_17)

target_link_libraries(juntos_exe PRIVATE juntos_lib)