cmake_minimum_required(VERSION 3.13)

set(BUILD_TESTING ON)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(radicle VERSION 0.1 DESCRIPTION "A C library containing common functionalities like database access or authentication using cookies to help grow REST API's.")

add_library(
	${PROJECT_NAME}
	SHARED
	""	
)	

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

if(BUILD_TESTING)
	enable_testing()
	include(FetchContent)
	FetchContent_Declare(
	  googletest
	  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
	)
	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)

	set(SUBHOOK_TESTS OFF)
	add_subdirectory(subhook)

	add_executable(
		radicle_test
		""
	)

	target_include_directories(
		radicle_test
		PUBLIC
	)

	target_link_libraries(
		radicle_test
		radicle
		subhook
		gtest_main
	)
	
	include(GoogleTest)
	gtest_discover_tests(radicle_test)
endif()

add_subdirectory(libcommon)
add_subdirectory(libpgdb)
add_subdirectory(libauth)


