macro(ccr_dependency PREFIX TAG DIGEST)
    set(${PREFIX}_TAG    "${TAG}")
    set(${PREFIX}_DIGEST "${DIGEST}")
    string(REGEX REPLACE "^v" "" ${PREFIX}_VERSION "${TAG}")
endmacro()

# renovate: datasource=github-tags packageName=cpm-cmake/CPM.cmake versioning=semver
ccr_dependency(CPM_DOWNLOAD      v0.43.1    456cb6754daaa010d57444d0c8ce6d95ecf006ab)
set(CPM_DOWNLOAD_SHA256 "1c40fc102ce9625d7de7eb14f541cab30cc3138dca627f0b0ec40293ce6c2934")
# renovate: datasource=github-tags packageName=nlohmann/json versioning=semver
ccr_dependency(NLOHMANN_JSON v3.12.0 55f93686c01528224f448c19128836e7df245f72)
# renovate: datasource=github-tags packageName=cucumber/messages versioning=semver
ccr_dependency(CUCUMBER_MESSAGES v34.2.0 073f951ce615149a1532185fbae013ad908a918e)
# renovate: datasource=github-tags packageName=cucumber/gherkin versioning=semver
ccr_dependency(CUCUMBER_GHERKIN  v42.0.0    f2f7b51ae6a886aefc1f4a2059e4989b83aab8c8)
# renovate: datasource=github-tags packageName=google/googletest versioning=semver
ccr_dependency(GOOGLE_TEST       v1.17.0    52eb8108c5bdec04579160ae17225d66034bd723)
# renovate: datasource=github-tags packageName=CLIUtils/CLI11 versioning=semver
ccr_dependency(CLI11             v2.6.2     37bb6edc5317e99af72ef48405e65d9ca5218861)
# renovate: datasource=github-tags packageName=fmtlib/fmt versioning=semver
ccr_dependency(LIBFMT            v12.1.0    407c905e45ad75fc29bf0f9bb7c5c2fd3475976f)
# renovate: datasource=github-tags packageName=zeux/pugixml versioning=semver
ccr_dependency(PUGIXML           v1.15      ee86beb30e4973f5feffe3ce63bfa4fbadf72f38)
# renovate: datasource=github-tags packageName=abseil/abseil-cpp
ccr_dependency(ABSEIL_CPP        20250814.2 0cf0a5c9d12cc3783363ab20f11613e69fd04c9a)
# renovate: datasource=github-tags packageName=google/re2
ccr_dependency(RE2               2025-08-12 0f6c07eae69151e606acb3d9232750c3442dff23)
# renovate: datasource=github-tags packageName=jbeder/yaml-cpp versioning=semver
ccr_dependency(YAML_CPP          v0.9.0     56e3bb550c91fd7005566f19c079cb7a503223cf)

if(CCR_FETCH_DEPS)
    if(NOT COMMAND CPMAddPackage)
        # ---------------------------------------------------------------------------
        # CPM – download on first configure if not already cached
        # ---------------------------------------------------------------------------
        set(CPM_USE_LOCAL_PACKAGES ON)
        set(CPM_DOWNLOAD_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

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

    CPMUsePackageLock("${CMAKE_CURRENT_LIST_DIR}/package-lock.cmake")

    # ---------------------------------------------------------------------------
    #
    # Dependencies
    #
    # ---------------------------------------------------------------------------

    # ---------------------------------------------------------------------------
    # nlohmann_json
    # ---------------------------------------------------------------------------
    CPMAddPackage(
        URI "gh:nlohmann/json@${NLOHMANN_JSON_VERSION}#${NLOHMANN_JSON_DIGEST}"
        NAME nlohmann_json
        OPTIONS
            "JSON_BuildTests Off"
            "JSON_Install ON"
    )

    # ---------------------------------------------------------------------------
    # cucumber_messages
    # ---------------------------------------------------------------------------
    CPMAddPackage(
        URI "gh:cucumber/messages@${CUCUMBER_MESSAGES_VERSION}#${CUCUMBER_MESSAGES_DIGEST}"
        NAME cucumber_messages
        SOURCE_SUBDIR cpp
    )

    # ---------------------------------------------------------------------------
    # cucumber_gherkin
    # ---------------------------------------------------------------------------
    CPMAddPackage(
        URI "gh:cucumber/gherkin@${CUCUMBER_GHERKIN_VERSION}#${CUCUMBER_GHERKIN_DIGEST}"
        NAME cucumber_gherkin
        SOURCE_SUBDIR cpp
    )

    # ---------------------------------------------------------------------------
    # GoogleTest
    # ---------------------------------------------------------------------------
    CPMAddPackage(
        URI "gh:google/googletest@${GOOGLE_TEST_VERSION}#${GOOGLE_TEST_DIGEST}"
        NAME googletest
        OPTIONS
            "INSTALL_GTEST OFF"
            "gtest_force_shared_crt ON"
    )

    if (TARGET gtest)
        set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES
            FOLDER External/GoogleTest
        )
        target_compile_options(gtest PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-character-conversion>)
        target_compile_options(gmock PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-character-conversion>)
    endif()

    # ---------------------------------------------------------------------------
    # cli11
    # ---------------------------------------------------------------------------
    CPMAddPackage(
        URI "gh:CLIUtils/CLI11@${CLI11_VERSION}#${CLI11_DIGEST}"
        NAME cli11
    )

    # ---------------------------------------------------------------------------
    # libfmt
    # ---------------------------------------------------------------------------
    CPMAddPackage(
        URI "gh:fmtlib/fmt@${LIBFMT_VERSION}#${LIBFMT_DIGEST}"
        NAME libfmt
    )

    # ---------------------------------------------------------------------------
    # pugixml
    # ---------------------------------------------------------------------------
    CPMAddPackage(
        URI "gh:zeux/pugixml@${PUGIXML_VERSION}#${PUGIXML_DIGEST}"
        NAME pugixml
    )

    if (CCR_USE_RE2)
        # ---------------------------------------------------------------------------
        # abseil-cpp
        # ---------------------------------------------------------------------------
        CPMAddPackage(
            URI "gh:abseil/abseil-cpp@${ABSEIL_CPP_VERSION}#${ABSEIL_CPP_DIGEST}"
            NAME abseil-cpp
            OPTIONS
                "ABSL_PROPAGATE_CXX_STD ON"
                "ABSL_ENABLE_INSTALL ON"
        )

        # ---------------------------------------------------------------------------
        # re2
        # ---------------------------------------------------------------------------
        CPMAddPackage(
            URI "gh:google/re2#${RE2_DIGEST}"
            NAME re2
            OPTIONS
                "RE2_BUILD_TESTING OFF"
        )
    endif()

    if (CCR_BUILD_TESTS)
        # ---------------------------------------------------------------------------
        # yaml-cpp
        # ---------------------------------------------------------------------------
        CPMAddPackage(
            URI "gh:jbeder/yaml-cpp@${YAML_CPP_VERSION}#${YAML_CPP_DIGEST}"
            NAME yaml-cpp
            OPTIONS
                "YAML_ENABLE_PIC OFF"
        )
    endif()
else()
    find_package(CLI11 ${CLI11_VERSION} REQUIRED)
    find_package(nlohmann_json ${NLOHMANN_JSON_VERSION} REQUIRED)
    find_package(cucumber_messages ${CUCUMBER_MESSAGES_VERSION} REQUIRED)
    find_package(cucumber_gherkin ${CUCUMBER_GHERKIN_VERSION} REQUIRED)
    find_package(GTest REQUIRED)
    find_package(pugixml ${PUGIXML_VERSION} REQUIRED)
    find_package(fmt ${LIBFMT_VERSION} REQUIRED)

    if (CCR_USE_RE2)
        find_package(absl ${ABSEIL_CPP_VERSION})
        find_package(re2)
    endif()

    if (CCR_BUILD_TESTS)
        find_package(yaml-cpp ${YAML_CPP_VERSION} REQUIRED)
    endif()
endif()
