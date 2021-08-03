find_library(JANSSON_LIBRARY NAMES jansson REQUIRED)

target_sources(
	radicle
	PRIVATE
		include/radicle/config.h
		src/config.c
)

target_include_directories(
	radicle
       	PUBLIC
       	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
	radicle	
	${JANSSON_LIBRARY}
)


if(BUILD_TESTING)
	#[[
	target_sources(
		radicle_test
		PRIVATE
			tests/include/radicle/tests/pgdb_hooks.hpp
			tests/src/pgdb_hooks.cpp
			tests/src/pgdb.cpp
	)

	target_include_directories(
		radicle_test
		PUBLIC	
		tests/include
	)
	]]#
endif()
