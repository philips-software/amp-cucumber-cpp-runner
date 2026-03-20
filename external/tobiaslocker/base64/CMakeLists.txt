FetchContent_Declare(
	tobiaslocker_base64
	GIT_REPOSITORY https://github.com/tobiaslocker/base64.git
	GIT_TAG 8d96a2a737ac1396304b1de289beb3a5ea0cb752
)

set(BASE64_ENABLE_TESTING OFF CACHE BOOL "Disable base64 unit tests" FORCE)
FetchContent_MakeAvailable(tobiaslocker_base64)

