add_library(cucumber_cpp.library.engine.test_helper.steps OBJECT)

target_link_libraries(cucumber_cpp.library.engine.test_helper.steps PUBLIC
    cucumber_cpp.library.engine
    gmock_main
)

target_sources(cucumber_cpp.library.engine.test_helper.steps PRIVATE
    StepImplementations.cpp
)

add_library(cucumber_cpp.library.engine.test_helper STATIC)

target_link_libraries(cucumber_cpp.library.engine.test_helper PUBLIC
    cucumber_cpp.library.engine
    gmock_main
)

target_sources(cucumber_cpp.library.engine.test_helper PRIVATE
    ContextManagerInstance.hpp
    FailureHandlerFixture.cpp
    FailureHandlerFixture.hpp
    TemporaryFile.cpp
    TemporaryFile.hpp
    TestExecutionInstance.hpp
    TestRunnerMock.hpp
)
