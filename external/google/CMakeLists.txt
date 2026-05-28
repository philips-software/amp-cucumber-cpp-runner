if (CCR_USE_RE2)
    add_subdirectory(abseil) # before re2
    add_subdirectory(re2)
endif()
add_subdirectory(googletest)
