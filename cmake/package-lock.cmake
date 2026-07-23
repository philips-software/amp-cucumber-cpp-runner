# CPM Package Lock
# This file should be committed to version control

# nlohmann_json
CPMDeclarePackage(nlohmann_json
  NAME nlohmann_json
  VERSION 3.12.0
  GIT_TAG 55f93686c01528224f448c19128836e7df245f72
  GITHUB_REPOSITORY nlohmann/json
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
  OPTIONS
    "JSON_BuildTests Off"
    "JSON_Install ON"
)
# cucumber_messages
CPMDeclarePackage(cucumber_messages
  NAME cucumber_messages
  VERSION 34.2.0
  GIT_TAG 073f951ce615149a1532185fbae013ad908a918e
  GITHUB_REPOSITORY cucumber/messages
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
  SOURCE_SUBDIR cpp
)
# cucumber_gherkin
CPMDeclarePackage(cucumber_gherkin
  NAME cucumber_gherkin
  VERSION 42.0.0
  GIT_TAG f2f7b51ae6a886aefc1f4a2059e4989b83aab8c8
  GITHUB_REPOSITORY cucumber/gherkin
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
  SOURCE_SUBDIR cpp
)
# googletest
CPMDeclarePackage(googletest
  NAME googletest
  VERSION 1.17.0
  GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723
  GITHUB_REPOSITORY google/googletest
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
  OPTIONS
    "INSTALL_GTEST OFF"
    "gtest_force_shared_crt ON"
)
# cli11
CPMDeclarePackage(cli11
  NAME cli11
  VERSION 2.6.2
  GIT_TAG 37bb6edc5317e99af72ef48405e65d9ca5218861
  GITHUB_REPOSITORY CLIUtils/CLI11
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# libfmt
CPMDeclarePackage(libfmt
  NAME libfmt
  VERSION 12.1.0
  GIT_TAG 407c905e45ad75fc29bf0f9bb7c5c2fd3475976f
  GITHUB_REPOSITORY fmtlib/fmt
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# pugixml
CPMDeclarePackage(pugixml
  NAME pugixml
  VERSION 1.15
  GIT_TAG ee86beb30e4973f5feffe3ce63bfa4fbadf72f38
  GITHUB_REPOSITORY zeux/pugixml
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# abseil-cpp
CPMDeclarePackage(abseil-cpp
  NAME abseil-cpp
  VERSION 20250814.2
  GIT_TAG 0cf0a5c9d12cc3783363ab20f11613e69fd04c9a
  GITHUB_REPOSITORY abseil/abseil-cpp
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
  OPTIONS
    "ABSL_PROPAGATE_CXX_STD ON"
    "ABSL_ENABLE_INSTALL ON"
)
# re2 (unversioned)
# CPMDeclarePackage(re2
#  NAME re2
#  GIT_TAG 0f6c07eae69151e606acb3d9232750c3442dff23
#  GITHUB_REPOSITORY google/re2
#  SYSTEM YES
#  EXCLUDE_FROM_ALL YES
#  OPTIONS
#    "RE2_BUILD_TESTING OFF"
#)
# yaml-cpp
CPMDeclarePackage(yaml-cpp
  NAME yaml-cpp
  VERSION 0.9.0
  GIT_TAG 56e3bb550c91fd7005566f19c079cb7a503223cf
  GITHUB_REPOSITORY jbeder/yaml-cpp
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
  OPTIONS
    "YAML_ENABLE_PIC OFF"
)
