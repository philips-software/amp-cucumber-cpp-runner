set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "")
set(CMAKE_COMPILE_WARNING_AS_ERROR Off)

add_subdirectory(nlohmann) # before cucumber

add_subdirectory(cliutils)
add_subdirectory(cucumber)
add_subdirectory(fmtlib)
add_subdirectory(google)
if (CCR_BUILD_TESTS)
    add_subdirectory(jbeder)
endif()
add_subdirectory(tobiaslocker)
add_subdirectory(zeux)

if (CCR_ENABLE_PARALLEL_SUPPORT)
    add_subdirectory(jbaldwin)
endif()
