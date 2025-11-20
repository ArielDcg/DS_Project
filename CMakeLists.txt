cmake_minimum_required(VERSION 3.10)
project(MazeVisualizer)
set(CMAKE_CXX_STANDARD 17)

# ===================================
# IMPORTANTE: Listar archivos explícitamente
# NO usar file(GLOB) para evitar incluir CollectorMazeGen.cpp
# ===================================
set(SRC_FILES
    src/main.cpp
    src/Grid.cpp
    src/Menu.cpp
    src/MazeSolver.cpp
    src/AStarSolver.cpp
    src/GreedySolver.cpp
    src/UCSSolver.cpp

    # ESTRUCTURA 1: Tabla Hash
    src/ChallengeSystem.cpp

    # ESTRUCTURA 2: Matriz Dispersa
    src/ExplorationHeatmap.cpp

    # ESTRUCTURA 3: Árbol AVL
    src/AlgorithmRankingAVL.cpp

    # Solver
    src/CollectorSolver.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(maze_app ${SRC_FILES})

# Use pkg-config to locate SFML
find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML_PKG REQUIRED IMPORTED_TARGET sfml-graphics)

# Add include dirs and link libraries
target_include_directories(maze_app PRIVATE ${SFML_PKG_INCLUDE_DIRS})
target_compile_options(maze_app PRIVATE ${SFML_PKG_CFLAGS_OTHER})
target_link_libraries(maze_app PRIVATE ${SFML_PKG_LIBRARIES})

# Copy assets to build directory
add_custom_command(TARGET maze_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            ${CMAKE_CURRENT_BINARY_DIR}/assets
)