add_library(cucumber_cpp.library.engine.test_helper.steps OBJECT)

target_link_libraries(cucumber_cpp.library.engine.test_helper.steps PUBLIC
    cucumber_cpp.library.engine
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.engine.test_helper.steps PRIVATE
    StepImplementations.cpp
)

add_library(cucumber_cpp.library.engine.test_helper STATIC)

target_link_libraries(cucumber_cpp.library.engine.test_helper PUBLIC
    cucumber_cpp.library.engine
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.engine.test_helper PRIVATE
    StepImplementations.cpp
    TemporaryFile.cpp
    TemporaryFile.hpp
)
