cmake_minimum_required(VERSION 3.2)

include(cmake/prelude.cmake)

project(
    juntos
    VERSION 0.1.0
    DESCRIPTION "A Peer-to-Peer Game Networking Library"
    HOMEPAGE_URL "https://github.com/nmstory/juntos"
    LANGUAGES CXX
)

# C++17 and above
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

# library
add_library(
    juntos_lib OBJECT
    src/client.cpp
    src/linux_handler.cpp
    src/session_linux.cpp
    src/session_windows.cpp
)

target_include_directories(
    juntos_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

# executable
add_executable(juntos_exe src/main.cpp)
add_executable(juntos::exe ALIAS juntos_exe)

set_property(TARGET juntos_exe PROPERTY OUTPUT_NAME juntos)

# link
if (WIN32)
    target_link_libraries(juntos_exe PRIVATE juntos_lib wsock32 ws2_32)
else()
    target_link_libraries(juntos_exe PRIVATE juntos_lib pthread)
endif()