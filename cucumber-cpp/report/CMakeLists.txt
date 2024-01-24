add_library(cucumber-cpp.report STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp.report PRIVATE
    JunitReport.cpp
    JunitReport.hpp
    Report.cpp
    Report.hpp
    StdOutReport.cpp
    StdOutReport.hpp
)

target_include_directories(cucumber-cpp.report PUBLIC
    ./../../
)

target_link_libraries(cucumber-cpp.report PUBLIC
    pugixml

    PRIVATE
    cucumber-cpp
)
