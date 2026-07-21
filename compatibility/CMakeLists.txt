add_library(compatibility.base)
target_sources(compatibility.base PRIVATE
    BaseCompatibility.cpp
    BaseCompatibility.hpp
)

target_link_libraries(compatibility.base PUBLIC
    cucumber_cpp
    GTest::gtest
)

target_include_directories(compatibility.base PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

function(add_compatibility_kit name)
    set(libname compatibility.${name}.test)
    add_executable(${libname})

    if(CCR_BUILD_TESTS)
        add_test(NAME ${libname} COMMAND ${libname})
    endif()

    target_link_libraries(${libname} PRIVATE
        GTest::gtest_main
        compatibility.base
    )

    target_include_directories(${libname} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    target_sources(${libname} PRIVATE
        TestCompatibility.cpp
    )


    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/${name}/${name}.cpp)
        target_sources(${libname} PRIVATE
            ${name}/${name}.cpp
        )
    else()
        set(SKIP_TEST On)
    endif()

    string(REPLACE "-" "_" KITNAME ${name})

    target_compile_definitions(${libname} PRIVATE
        KIT_NAME=${KITNAME}
        KIT_STRING="${name}"
        KIT_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/${name}"
        KIT_NDJSON_FILE="${CMAKE_CURRENT_SOURCE_DIR}/${name}/${name}.ndjson"
        $<$<BOOL:${SKIP_TEST}>:SKIP_TEST>
    )
endfunction()

file(GLOB kits RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
foreach(kit ${kits})
    if (IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${kit})
        add_compatibility_kit(${kit})
    endif()
endforeach()
