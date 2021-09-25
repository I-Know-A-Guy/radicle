
find_library(ULFIUS_LIBRARY NAMES ulfius REQUIRED)
find_library(JANSSON_LIBRARY NAMES jansson REQUIRED)

# http://www.pcre.org/ Make sure to download version 2
find_library(PCRE2_LIBRARY NAMES "pcre2-8" REQUIRED) 

target_sources(
	radicle
	PRIVATE
	include/radicle/api/mail/sendgrid.h		
	src/mail/sendgrid.c
	include/radicle/api/instance.h
	src/instance.c
	include/radicle/api/json_validate.h
	src/json_validate.c
	include/radicle/api/endpoints/internal_codes.h
	src/endpoints/internal_codes.c
	include/radicle/api/endpoints/endpoint.h
	src/endpoints/endpoint.c
	include/radicle/api/endpoints/auth.h
	src/endpoints/auth.c
)

target_include_directories(
	radicle
       	PUBLIC
	include
)

target_link_libraries(
	${PROJECT_NAME}
	${ULFIUS_LIBRARY}
	${JANSSON_LIBRARY}
	${PCRE2_LIBRARY}
)
if(BUILD_TESTING)
	target_sources(
		radicle_test
		PRIVATE
			tests/include/radicle/tests/api/api_fixture.hpp
			tests/src/endpoints/endpoint.cpp
	)

	target_include_directories(
		${PROJECT_NAME}_test
		PUBLIC
		tests/include
	)
endif()
