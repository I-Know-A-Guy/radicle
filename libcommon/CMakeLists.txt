target_sources(
	radicle
	PRIVATE
		include/radicle/types/uuid.h	
		src/types/uuid.c
		include/radicle/types/string.h	
		src/types/string.c
		include/radicle/types/linked_list.h
		src/types/linked_list.c
		include/radicle/print.h
)

target_include_directories(
	radicle
	PUBLIC
	include
)

if(BUILD_TESTING)

	# TODO if statement for either adding subhook_unix.c or subhook_windows.c depending on building platform
	target_sources(
		radicle_test
		PRIVATE
			tests/include/radicle/tests/radicle_fixture.hpp
			tests/src/types/uuid.cpp
			tests/src/types/string.cpp
			tests/src/types/linked_list.cpp
	)

	target_include_directories(
		radicle_test
		PUBLIC
		tests/include
	)
endif()
