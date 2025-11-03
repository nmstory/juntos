# executable
add_executable(juntos_echo main.cpp)
target_link_libraries(juntos_echo PRIVATE juntos_lib)

# link
if (WIN32)
    target_link_libraries(juntos_echo PRIVATE juntos_lib wsock32 ws2_32)
else()
    target_link_libraries(juntos_echo PRIVATE juntos_lib pthread)
endif()