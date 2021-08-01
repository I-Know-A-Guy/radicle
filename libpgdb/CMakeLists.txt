find_package(PostgreSQL 12 REQUIRED)

target_sources(
	radicle
	PRIVATE
		include/radicle/pgdb.h
		src/pgdb.c
)

target_include_directories(
	radicle
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
)

if(BUILD_TESTING)
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
endif()
