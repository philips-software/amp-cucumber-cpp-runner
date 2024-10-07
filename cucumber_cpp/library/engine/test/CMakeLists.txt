add_executable(cucumber_cpp.library.engine.test)
add_test(NAME cucumber_cpp.library.engine.test COMMAND cucumber_cpp.library.engine.test)

target_link_libraries(cucumber_cpp.library.engine.test PUBLIC
    cucumber_cpp.library.engine
    gmock_main
)

target_sources(cucumber_cpp.library.engine.test PRIVATE
    TestStep.cpp
    TestStringTo.cpp
)
