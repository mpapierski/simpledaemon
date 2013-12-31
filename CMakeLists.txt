cmake_minimum_required (VERSION 2.6)
project (simpledeamon)

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

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
install (TARGETS simpledaemon
	DESTINATION bin)

# "make docker"
add_custom_target (docker
	COMMAND cpack -G DEB .
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Dockerfile ${CMAKE_CURRENT_BINARY_DIR}/Dockerfile
	COMMAND /usr/bin/docker rmi simpledaemon || /bin/true
	COMMAND /usr/bin/docker build -t simpledaemon ${CMAKE_CURRENT_BINARY_DIR})

# Packaging
set (CPACK_PACKAGE_CONTACT "michal@papierski.net")
include (CPack)