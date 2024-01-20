
add_library(cucumber-cpp-runner ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-runner PRIVATE
    main.cpp
)

target_link_libraries(cucumber-cpp-runner PUBLIC
    cucumber_gherkin_lib
    cucumber-cpp
    cucumber-cpp.report
    gtest
    gmock

)
