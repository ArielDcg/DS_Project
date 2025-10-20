cmake_minimum_required(VERSION 3.10)
project(MazeVisualizer)
set(CMAKE_CXX_STANDARD 17)

# collect sources
file(GLOB SRC_FILES src/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(maze_app ${SRC_FILES})

# Use pkg-config to locate SFML (works with distro packages that install .pc files)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML_PKG REQUIRED IMPORTED_TARGET sfml-graphics)

# Add include dirs and link libraries to the target
target_include_directories(maze_app PRIVATE ${SFML_PKG_INCLUDE_DIRS})
target_compile_options(maze_app PRIVATE ${SFML_PKG_CFLAGS_OTHER})
target_link_libraries(maze_app PRIVATE ${SFML_PKG_LIBRARIES})
