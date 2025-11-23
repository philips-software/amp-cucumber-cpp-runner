add_library(cucumber_cpp.library.report STATIC ${CCR_EXCLUDE_FROM_ALL})

configure_file(
    Environment.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/g/Environment.h"
)

target_sources(cucumber_cpp.library.report PRIVATE
    JunitReport.cpp
    JunitReport.hpp
    NdjsonReport.cpp
    NdjsonReport.hpp
    Report.cpp
    Report.hpp
    StdOutReport.cpp
    StdOutReport.hpp
)

target_include_directories(cucumber_cpp.library.report PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/g")

target_include_directories(cucumber_cpp.library.report PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.report PUBLIC
    pugixml::pugixml
    cucumber_cpp.library
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test_helper)
endif()
