# executable
add_executable(juntos_position_replication main.cpp)
target_link_libraries(juntos_position_replication PRIVATE juntos_lib SDL3::SDL3)