#pragma once
#include "Grid.h"
#include <stack>
#include <vector>

class MazeSolver {
public:
    MazeSolver(Grid &grid, Coord start, Coord goal);

    // Ejecuta un paso del algoritmo DFS
    bool step();

    // Indica si se encontró el camino
    bool finished() const;

    // Devuelve la celda actual en el camino
    bool getCurrent(Coord &out) const;

    // Devuelve el camino completo si se resolvió
    const std::vector<Coord> &getSolution() const;

    int getNodesExplored() const { return nodesExplored; }

private:
    Grid &g;
    Coord start;
    Coord goal;
    std::stack<Coord> path;
    std::vector<std::vector<bool>> visited;
    std::vector<std::vector<Coord>> cameFrom;
    std::vector<Coord> solution;
    bool solved = false;

    int nodesExplored = 0;
};
