project(radicle_pgdb)

find_package(PostgreSQL 13 REQUIRED)

add_library(
	${PROJECT_NAME}
       	STATIC
       	src/pgdb.c
)

target_include_directories(
	${PROJECT_NAME}
       	PUBLIC
       	${PostgreSQL_INCLUDE_DIRS}
       	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_directories(
	${PROJECT_NAME}
       	PUBLIC
       	${PostgreSQL_LIBRARY_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
       	${PostgreSQL_LIBRARIES}
	radicle_common
)

add_library(
	radicle_pgdb_testlib
	STATIC
	tests/src/pgdb_hooks.cpp
)

target_include_directories(
	radicle_pgdb_testlib
	PUBLIC	
       	${CMAKE_CURRENT_SOURCE_DIR}/tests/include
)

target_link_libraries(
	radicle_pgdb_testlib
	gtest
	${PROJECT_NAME}
	radicle_common_testlib
)

add_executable(
	"${PROJECT_NAME}_test"
	tests/src/pgdb.cpp
)

target_link_libraries(
	"${PROJECT_NAME}_test"
	${PROJECT_NAME}
	gtest_main
)

gtest_discover_tests(${PROJECT_NAME}_test)

