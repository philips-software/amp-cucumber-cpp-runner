macro(ccr_dependency PREFIX TAG DIGEST)
    set(${PREFIX}_TAG    "${TAG}")
    set(${PREFIX}_DIGEST "${DIGEST}")
    string(REGEX REPLACE "^v" "" ${PREFIX}_VERSION "${TAG}")
endmacro()

macro(ccr_cucumber_package cucumber_package)
    file(STRINGS "${PROJECT_SOURCE_DIR}/dependencies/${cucumber_package}.version"
        ccr_cucumber_dependency_version LIMIT_COUNT 1)
    string(REGEX MATCH "^([^@ ]+)@([^ ]+)" ccr_cucumber_dependency_match
        "${ccr_cucumber_dependency_version}")
    if (NOT ccr_cucumber_dependency_match)
        message(FATAL_ERROR "Invalid dependency version file for ${cucumber_package}")
    endif()

    set(ccr_cucumber_git_repo "${CMAKE_MATCH_1}")
    set(ccr_cucumber_git_hash "${CMAKE_MATCH_2}")
    string(REPLACE "-" "_" ccr_cucumber_package_name "${cucumber_package}")
    CPMAddPackage(
        URI "gh:${ccr_cucumber_git_repo}#${ccr_cucumber_git_hash}"
        ${ARGN}
    )
endmacro()

ccr_dependency(CPM_DOWNLOAD  v0.43.1    456cb6754daaa010d57444d0c8ce6d95ecf006ab)
ccr_dependency(NLOHMANN_JSON v3.12.0    55f93686c01528224f448c19128836e7df245f72)
ccr_dependency(GOOGLE_TEST   v1.17.0    52eb8108c5bdec04579160ae17225d66034bd723)
ccr_dependency(CLI11         v2.6.2     37bb6edc5317e99af72ef48405e65d9ca5218861)
ccr_dependency(LIBFMT        v12.1.0    407c905e45ad75fc29bf0f9bb7c5c2fd3475976f)
ccr_dependency(PUGIXML       v1.15      ee86beb30e4973f5feffe3ce63bfa4fbadf72f38)
ccr_dependency(ABSEIL_CPP    20250814.2 0cf0a5c9d12cc3783363ab20f11613e69fd04c9a)
ccr_dependency(RE2           2025-08-12 0f6c07eae69151e606acb3d9232750c3442dff23)

if((CCR_FETCH_DEPS OR CCR_BUILD_TESTS) AND NOT COMMAND CPMAddPackage)
    # ---------------------------------------------------------------------------
    # CPM – download on first configure if not already cached
    # ---------------------------------------------------------------------------
    set(CPM_USE_LOCAL_PACKAGES ON)
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    set(CPM_DOWNLOAD_SHA256 "1c40fc102ce9625d7de7eb14f541cab30cc3138dca627f0b0ec40293ce6c2934")

    if(NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
        message(STATUS "Downloading CPM.cmake ${CPM_DOWNLOAD_VERSION}…")
        file(DOWNLOAD
            "https://github.com/cpm-cmake/CPM.cmake/releases/download/${CPM_DOWNLOAD_TAG}/CPM.cmake"
            "${CPM_DOWNLOAD_LOCATION}"
            TLS_VERIFY ON
            EXPECTED_HASH SHA256=${CPM_DOWNLOAD_SHA256}
            STATUS CPM_DOWNLOAD_STATUS
        )

        list(GET CPM_DOWNLOAD_STATUS 0 CPM_DOWNLOAD_STATUS_code)
        if(NOT CPM_DOWNLOAD_STATUS_code EQUAL 0)
            message(FATAL_ERROR "Failed to download CPM.cmake: ${CPM_DOWNLOAD_STATUS}")
        endif()
    endif()

    include("${CPM_DOWNLOAD_LOCATION}")
endif()

if(CCR_FETCH_DEPS)
    # ---------------------------------------------------------------------------
    #
    # Dependencies
    #
    # ---------------------------------------------------------------------------

    # ---------------------------------------------------------------------------
    # nlohmann_json
    # ---------------------------------------------------------------------------
    if(NOT TARGET nlohmann_json::nlohmann_json)
        CPMAddPackage(
            URI "gh:nlohmann/json@${NLOHMANN_JSON_VERSION}#${NLOHMANN_JSON_DIGEST}"
            NAME nlohmann_json
            OPTIONS
                "JSON_BuildTests Off"
                "JSON_Install ON"
        )
    endif()

    # ---------------------------------------------------------------------------
    # GoogleTest
    # ---------------------------------------------------------------------------
    if(NOT TARGET GTest::gtest)
        CPMAddPackage(
            URI "gh:google/googletest@${GOOGLE_TEST_VERSION}#${GOOGLE_TEST_DIGEST}"
            NAME googletest
            OPTIONS
                "INSTALL_GTEST OFF"
                "gtest_force_shared_crt ON"
        )
    endif()

    if (TARGET GTest::gtest)
        set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES
            FOLDER External/GoogleTest
        )
        target_compile_options(gtest PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-character-conversion>)
        target_compile_options(gmock PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-character-conversion>)
    endif()

    # ---------------------------------------------------------------------------
    # cli11
    # ---------------------------------------------------------------------------
    if(NOT TARGET CLI11::CLI11)
        CPMAddPackage(
            URI "gh:CLIUtils/CLI11@${CLI11_VERSION}#${CLI11_DIGEST}"
            NAME cli11
        )
    endif()

    # ---------------------------------------------------------------------------
    # libfmt
    # ---------------------------------------------------------------------------
    if(NOT TARGET fmt::fmt)
        CPMAddPackage(
            URI "gh:fmtlib/fmt@${LIBFMT_VERSION}#${LIBFMT_DIGEST}"
            NAME fmt
            OPTIONS
                "FMT_INSTALL ON"
        )
    endif()

    # ---------------------------------------------------------------------------
    # pugixml
    # ---------------------------------------------------------------------------
    if(NOT TARGET pugixml::pugixml)
        CPMAddPackage(
            URI "gh:zeux/pugixml@${PUGIXML_VERSION}#${PUGIXML_DIGEST}"
            NAME pugixml
        )
    endif()

    if (CCR_USE_RE2)
        # ---------------------------------------------------------------------------
        # abseil-cpp
        # ---------------------------------------------------------------------------
        if(NOT TARGET absl::base)
            CPMAddPackage(
                URI "gh:abseil/abseil-cpp@${ABSEIL_CPP_VERSION}#${ABSEIL_CPP_DIGEST}"
                NAME abseil-cpp
                OPTIONS
                    "ABSL_PROPAGATE_CXX_STD ON"
                    "ABSL_ENABLE_INSTALL ON"
            )
        endif()

        # ---------------------------------------------------------------------------
        # re2
        # ---------------------------------------------------------------------------
        if(NOT TARGET re2::re2)
            CPMAddPackage(
                URI "gh:google/re2#${RE2_DIGEST}"
                NAME re2
                OPTIONS
                    "RE2_BUILD_TESTING OFF"
            )
        endif()
    endif()

    ccr_cucumber_package(messages SOURCE_SUBDIR cpp)
    ccr_cucumber_package(gherkin SOURCE_SUBDIR cpp)
    ccr_cucumber_package(query SOURCE_SUBDIR cpp)
    ccr_cucumber_package(tag-expressions SOURCE_SUBDIR cpp)
    ccr_cucumber_package(pretty-formatter SOURCE_SUBDIR cpp)
    ccr_cucumber_package(cucumber-expressions SOURCE_SUBDIR cpp)
else()
    find_package(CLI11 REQUIRED)
    find_package(nlohmann_json REQUIRED)
    find_package(GTest REQUIRED)
    find_package(pugixml REQUIRED)
    find_package(fmt 10 REQUIRED)

    find_package(cucumber_messages REQUIRED)
    find_package(cucumber_gherkin REQUIRED)
    find_package(cucumber_query REQUIRED)
    find_package(cucumber_tag_expressions REQUIRED)
    find_package(cucumber_pretty_formatter REQUIRED)
    find_package(cucumber_cucumber_expressions REQUIRED)
endif()

if(CCR_BUILD_TESTS)
    # ---------------------------------------------------------------------------
    # Cucumber Compatibility Kit (feature files only, no CMake project)
    # ---------------------------------------------------------------------------
    ccr_cucumber_package(compatibility-kit DOWNLOAD_ONLY YES)
endif()
