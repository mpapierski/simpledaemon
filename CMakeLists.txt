cmake_minimum_required (VERSION 2.6)
project (simpledeamon)

find_package (Boost REQUIRED COMPONENTS
	program_options
	system)
find_package (Threads REQUIRED)
find_package (PythonInterp REQUIRED)

# Generate build info
set (BUILD_INFO_HDR ${CMAKE_CURRENT_BINARY_DIR}/build_info.hpp)
add_custom_command (OUTPUT ${BUILD_INFO_HDR}
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_build_info.py ${BUILD_INFO_HDR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_build_info.py)
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIRS})
add_executable (simpledaemon
	${BUILD_INFO_HDR}
	src/main.cpp)
target_link_libraries (simpledaemon
	${Boost_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT})