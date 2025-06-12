set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "")

add_subdirectory(nlohmann) # before cucumber

add_subdirectory(cliutils)
add_subdirectory(cucumber)
add_subdirectory(googletest)
add_subdirectory(jbeder)
add_subdirectory(zeux)
