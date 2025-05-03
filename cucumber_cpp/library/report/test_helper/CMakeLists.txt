add_library(cucumber_cpp.library.report.test_helper INTERFACE)

target_link_libraries(cucumber_cpp.library.report.test_helper INTERFACE
    cucumber_cpp.library.report
    gmock_main
)

target_sources(cucumber_cpp.library.report.test_helper INTERFACE
    ReportForwarderMock.hpp
)
