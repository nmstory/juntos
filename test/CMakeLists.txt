# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(juntosTests LANGUAGES CXX)

# ---- Tests ----

add_executable(juntos_test source/juntos_test.cpp)
target_link_libraries(juntos_test PRIVATE juntos_lib)
target_compile_features(juntos_test PRIVATE cxx_std_17)

add_test(NAME juntos_test COMMAND juntos_test)

# ---- End-of-file commands ----

add_folders(Test)
