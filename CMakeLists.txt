cmake_minimum_required(VERSION 3.10)
project(MazeVisualizer)
set(CMAKE_CXX_STANDARD 17)

# collect sources (esto ya incluye RankingTree.cpp si está en src/)
file(GLOB SRC_FILES src/*.cpp)

# incluye headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# ejecutable principal
add_executable(maze_app ${SRC_FILES})

# SFML via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML_PKG REQUIRED IMPORTED_TARGET sfml-graphics)

target_include_directories(maze_app PRIVATE ${SFML_PKG_INCLUDE_DIRS})
target_compile_options(maze_app PRIVATE ${SFML_PKG_CFLAGS_OTHER})
target_link_libraries(maze_app PRIVATE ${SFML_PKG_LIBRARIES})

# copiar assets automáticamente
add_custom_command(TARGET maze_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            ${CMAKE_CURRENT_BINARY_DIR}/assets
)
