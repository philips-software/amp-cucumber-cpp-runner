add_library(cucumber-cpp.report STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp.report PRIVATE
    JsonReport.cpp
    JsonReport.hpp
    JunitReport.cpp
    JunitReport.hpp
    Report.hpp
    StdOutReport.cpp
    StdOutReport.hpp
)

target_include_directories(cucumber-cpp.report PUBLIC
    ./../../
)

target_link_libraries(cucumber-cpp.report PUBLIC
    nlohmann_json
    pugixml

    PRIVATE
    cucumber-cpp
)
