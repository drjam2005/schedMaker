cmake_minimum_required(VERSION 3.28)
project(scheduler)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(exec Scheduler)
set(src
	src/main.cpp
	src/objects.cpp
	src/renderer.cpp
	src/scheduler.cpp
	vendor/fonts/fontBold.o
	vendor/fonts/fontReg.o
)

add_executable(${exec} ${src})
target_include_directories(${exec} PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

if(WIN32) 
	target_include_directories(${exec} PRIVATE
		${CMAKE_SOURCE_DIR}/vendor/raylib/windows/include/
	)
	target_link_directories(${exec} PRIVATE
		${CMAKE_SOURCE_DIR}/vendor/raylib/windows/lib/
	)
	target_link_libraries(${exec} PRIVATE
		libraylib.a
		gdi32
		winmm
	)
else()
	target_include_directories(${exec} PRIVATE
		${CMAKE_SOURCE_DIR}/vendor/raylib/unix/include/
	)
	target_link_directories(${exec} PRIVATE
		${CMAKE_SOURCE_DIR}/vendor/raylib/unix/lib/
	)
	target_link_libraries(${exec} PRIVATE
		libraylib.a
		X11
	)
endif()

