cmake_minimum_required(VERSION 3.10)

project(YetAnotherJVSEmu)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 sdl2)

include_directories(${THREAD_INLCUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/Includes)

add_executable(${PROJECT_NAME}
	main.cpp
	Includes/JvsIo.cpp
	Includes/SerIo.cpp
	Includes/GpIo.cpp
	Includes/SdlIo.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 14
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${SDL2_LIBRARIES} bcm2835)
