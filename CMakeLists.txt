cmake_minimum_required(VERSION 3.10)

project(YAJVSEmu-Dump)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Git)

pkg_check_modules(SERIAL REQUIRED libserialport)

include_directories(${THREAD_INLCUDE_DIRS} ${SERIAL_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/Includes)

add_executable(${PROJECT_NAME}
	main.cpp
	Includes/JvsIo.cpp
	Includes/SerIo.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
)

target_compile_options(${PROJECT_NAME} PRIVATE -march=native -Wsign-conversion -Wall -Wextra -Wpedantic)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${SERIAL_LIBRARIES})
