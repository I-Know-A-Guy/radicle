project(radicle_common)

add_library(
	${PROJECT_NAME}
       	STATIC
       	src/types/string.c
       	src/types/uuid.c
)
target_include_directories(
	${PROJECT_NAME}
       	PUBLIC
       	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(
	radicle_test_lib
	STATIC
	tests/src/subhook.c
	tests/src/subhook_unix.c
	tests/src/subhook_x86.c
)

target_include_directories(
	radicle_test_lib
	PUBLIC	
       	${CMAKE_CURRENT_SOURCE_DIR}/tests/include
)

add_executable(
	"${PROJECT_NAME}_test"
	tests/src/types/string.cpp
	tests/src/types/uuid.cpp
)

target_link_libraries(
	"${PROJECT_NAME}_test"
	${PROJECT_NAME}
	radicle_test_lib
	gtest_main
)

gtest_discover_tests(${PROJECT_NAME}_test)

