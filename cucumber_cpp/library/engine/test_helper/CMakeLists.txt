add_library(cucumber_cpp.library.engine.test_helper INTERFACE)

target_link_libraries(cucumber_cpp.library.engine.test_helper INTERFACE
    cucumber_cpp.library.engine
    gmock_main
)

target_sources(cucumber_cpp.library.engine.test_helper INTERFACE
    ContextManagerInstance.hpp
    TestRunnerMock.hpp
)
