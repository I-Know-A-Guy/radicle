find_package(OpenSSL REQUIRED)
# https://github.com/P-H-C/phc-winner-argon2
find_library(argon2 argon2 REQUIRED)

target_sources(
	radicle
	PRIVATE
		include/radicle/auth/crypto.h
		src/auth/crypto.c
		include/radicle/auth/types.h
		src/auth/types.c
		include/radicle/auth/db.h
		src/auth/db.c
		include/radicle/auth.h
		src/auth.c
)

target_include_directories(
	radicle
       	PUBLIC
       	${OPENSSL_INCLUDE_DIR}
       	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
	${PROJECT_NAME}
       	${OPENSSL_LIBRARIES}
       	${argon2}
)

if(BUILD_TESTING)
	target_sources(
		radicle_test
		PRIVATE
			tests/include/radicle/tests/auth/auth_fixture.hpp
			tests/src/crypto.cpp
			tests/src/db.cpp
	)

	target_include_directories(
		${PROJECT_NAME}_test
		PUBLIC
		tests/include
	)
endif()
