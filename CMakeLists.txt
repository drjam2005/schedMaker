cmake_minimum_required(VERSION 3.28)
project(scheduler)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(exec exec)
set(src
	src/main.cpp
	src/objects.cpp
	src/renderer.cpp
	src/scheduler.cpp
)

add_executable(${exec} ${src})

target_include_directories(${exec} PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_directories(${exec} PRIVATE
	${CMAKE_SOURCE_DIR}/lib
)
target_link_libraries(${exec} PRIVATE
	raylib
)
