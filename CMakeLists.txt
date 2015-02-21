cmake_minimum_required(VERSION 2.4)

project(axcoro C)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(COMMON_CFLAGS "${COMMON_CFLAGS} -DAXIA_SYS_LINUX")
else()
	if(WIN32)
		set(COMMON_CFLAGS "${COMMON_CFLAGS} -DAXIA_SYS_WINDOWS")
	endif()
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -std=gnu99")
	set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE}     -O2")
	set(CMAKE_C_FLAGS_DEBUG          "${CMAKE_C_FLAGS_DEBUG}       -O0 -g")
endif()


add_library(axcoro STATIC ax-coro-linux.c)

add_executable(axcoro-test test.c)
target_link_libraries(axcoro-test axcoro)