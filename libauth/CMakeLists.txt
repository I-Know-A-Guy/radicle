project(radicle_auth)

find_package(OpenSSL REQUIRED)
find_package(PostgreSQL 13 REQUIRED)

# https://github.com/P-H-C/phc-winner-argon2
find_library(argon2 argon2 REQUIRED)

add_library(
	${PROJECT_NAME}
       	STATIC
	src/auth/crypto.c
	src/auth/db.c
	src/auth/setup.c
	src/auth/types.c
	src/auth.c
)

target_include_directories(
	${PROJECT_NAME}
       	PUBLIC
       	${PostgreSQL_INCLUDE_DIRS}
       	${OPENSSL_INCLUDE_DIR}
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
       	${OPENSSL_LIBRARIES}
       	${argon2}
	radicle_common
	radicle_pgdb
)

add_executable(
	${PROJECT_NAME}_test
	tests/src/crypto.cpp
	tests/src/auth_fixture.cpp
	tests/src/db.cpp
	tests/src/auth.cpp
)

target_include_directories(
	${PROJECT_NAME}_test
	PUBLIC
	tests/include
)

target_link_libraries(
	${PROJECT_NAME}_test
	${PROJECT_NAME}
	radicle_common_testlib
	radicle_pgdb_testlib
	gtest_main
)

gtest_discover_tests(${PROJECT_NAME}_test)
