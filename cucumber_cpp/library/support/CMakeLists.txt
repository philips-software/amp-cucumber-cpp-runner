add_library(cucumber_cpp.library.support STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.support PRIVATE
    Duration.hpp
    Duration.cpp
    Join.cpp
    Join.hpp
    SupportCodeLibrary.cpp
    SupportCodeLibrary.hpp
    Timestamp.cpp
    Timestamp.hpp
    Types.hpp
)

target_include_directories(cucumber_cpp.library.support PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.support PUBLIC
    cucumber_cpp.library
    cucumber_cpp.library.util
)

if (CCR_BUILD_TESTS)
    # add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
