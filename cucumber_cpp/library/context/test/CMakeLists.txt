add_executable(cucumber_cpp.library.context.test)
gtest_discover_tests(
    cucumber_cpp.library.context.test
    DISCOVERY_MODE PRE_TEST
    DISCOVERY_TIMEOUT 30
)

target_link_libraries(cucumber_cpp.library.context.test PUBLIC
    cucumber_cpp.library.context
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.context.test PRIVATE
    TestContext.cpp
)

foreach(accessor IN ITEMS GetOrElse GetOrElseAt GetOrEmplaceWith GetOrEmplaceAtWith GetOrEmplaceAsWith)
    if(accessor STREQUAL "GetOrEmplaceAsWith")
        set(factory_return_type_template_parameter U)
    else()
        set(factory_return_type_template_parameter T)
    endif()

    foreach(factory_return_type IN ITEMS Convertible Reference)
        set(test_target cucumber_cpp.library.context.test.${accessor}Factory${factory_return_type})
        add_library(${test_target} OBJECT EXCLUDE_FROM_ALL
            ${accessor}Factory${factory_return_type}.cpp
        )
        target_link_libraries(${test_target} PRIVATE cucumber_cpp.library.context)

        add_test(
            NAME TestContext.${accessor}Rejects${factory_return_type}Factory
            COMMAND ${CMAKE_COMMAND}
                "-DBUILD_DIRECTORY=${CMAKE_BINARY_DIR}"
                "-DBUILD_CONFIG=$<CONFIG>"
                "-DTEST_TARGET=${test_target}"
                "-DEXPECTED_DIAGNOSTIC=${accessor} factory must return exactly ${factory_return_type_template_parameter}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/ExpectCompileFailure.cmake"
        )
        # Each test invokes the build system on the shared build tree; concurrent invocations corrupt its state.
        set_tests_properties(TestContext.${accessor}Rejects${factory_return_type}Factory PROPERTIES RESOURCE_LOCK build_tree)
    endforeach()
endforeach()
