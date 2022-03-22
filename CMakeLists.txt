cmake_minimum_required(VERSION 3.10)

project(YAJVSEmu LANGUAGES CXX)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Git)

# https://www.airspayce.com/mikem/bcm2835/
find_library(BCM2835_LIB bcm2835 REQUIRED)

pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(XWII REQUIRED libxwiimote)
pkg_check_modules(SERIAL REQUIRED libserialport)

if(GIT_FOUND)
	execute_process(COMMAND "${GIT_EXECUTABLE}" describe --always --tags --first-parent
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else()
	set(GIT_VERSION "unknown")
endif()

configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${PROJECT_SOURCE_DIR}/version.h" @ONLY NEWLINE_STYLE LF)

include_directories(${THREAD_INLCUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${XWII_INCLUDE_DIRS} ${SERIAL_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/Includes)

add_executable(${PROJECT_NAME}
	main.cpp
	Includes/Setup.cpp
	Includes/JvsIo.cpp
	Includes/SerIo.cpp
	Includes/GpIo.cpp
	Includes/SdlIo.cpp
	Includes/WiiIo.cpp
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	include(CheckIPOSupported)
	check_ipo_supported(RESULT supported OUTPUT error)
	if (supported)
		set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14)

target_compile_options(${PROJECT_NAME} PRIVATE -march=native -Wsign-conversion -Wall -Wextra -Wpedantic)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${SDL2_LIBRARIES} ${SERIAL_LIBRARIES} ${XWII_LIBRARIES} ${BCM2835_LIB})
