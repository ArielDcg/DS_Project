#ifndef UCS_SOLVER_H
#define UCS_SOLVER_H

#include "Grid.h"
#include <vector>
#include <queue>
#include <limits>

class UCSSolver {
public:
    UCSSolver(Grid &grid, Coord start, Coord goal);

    bool step();

    bool finished() const;
    bool getCurrent(Coord &out) const;
    const std::vector<Coord> &getSolution() const;

    enum CellState { UNKNOWN = 0, OPEN = 1, CLOSED = 2 };
    const std::vector<std::vector<CellState>> &getStateGrid() const { return stateGrid; }
    const std::vector<std::vector<float>> &getGScoreGrid() const { return gScore; }

private:
    Grid &g;
    Coord start;
    Coord goal;

    std::vector<std::vector<CellState>> stateGrid;
    std::vector<std::vector<bool>> closed;
    std::vector<std::vector<float>> gScore;
    std::vector<std::vector<Coord>> cameFrom;

    struct PQNode { float cost; int id; int x; int y; };
    struct Compare {
        bool operator()(const PQNode &a, const PQNode &b) const {
            if (a.cost == b.cost) return a.id > b.id;
            return a.cost > b.cost;
        }
    };
    std::priority_queue<PQNode, std::vector<PQNode>, Compare> openPQ;
    int pushCounter = 0;

    bool done = false;
    bool solved = false;
    std::vector<Coord> solution;
    Coord current;

    void reconstruct_path(int x, int y);
};

#endif // UCS_SOLVER_H
