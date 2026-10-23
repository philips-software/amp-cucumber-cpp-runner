add_executable(cucumber_cpp.library.support.test)
add_test(NAME cucumber_cpp.library.support.test COMMAND cucumber_cpp.library.support.test)

target_link_libraries(cucumber_cpp.library.support.test PUBLIC
    cucumber_cpp.library.support
    GTest::gmock
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.support.test PRIVATE
    TestDummy.cpp
    TestParameterMap.cpp
)

if (CCR_HAS_RE2)
    target_link_libraries(cucumber_cpp.library.support.test PRIVATE
        re2::re2
    )

    target_compile_definitions(cucumber_cpp.library.support.test PRIVATE
        CCR_HAS_RE2
    )
endif()
