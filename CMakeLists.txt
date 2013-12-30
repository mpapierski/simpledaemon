cmake_minimum_required (VERSION 2.6)
project (simpledeamon)

find_package (Boost REQUIRED COMPONENTS
	program_options
	system)
find_package (Threads REQUIRED)
include_directories (${Boost_INCLUDE_DIRS})
add_executable (simpledaemon
	src/main.cpp)
target_link_libraries (simpledaemon
	${Boost_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT})