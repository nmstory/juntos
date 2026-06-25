project(juntosTests LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(juntos_test source/juntos_test.cpp)
target_link_libraries(juntos_test PRIVATE juntos_lib GTest::gtest_main)
target_compile_features(juntos_test PRIVATE cxx_std_20)

gtest_discover_tests(juntos_test)
