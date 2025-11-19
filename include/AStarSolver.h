#ifndef ASTAR_SOLVER_H
#define ASTAR_SOLVER_H

#include "Grid.h"
#include <vector>
#include <queue>
#include <limits>

class AStarSolver {
public:
    AStarSolver(Grid &grid, Coord start, Coord goal);

    // one atomic expansion step (returns true when finished)
    bool step();

    bool finished() const;
    bool getCurrent(Coord &out) const;
    const std::vector<Coord> &getSolution() const;

    // NEW: visualization helpers
    enum CellState { UNKNOWN = 0, OPEN = 1, CLOSED = 2 };
    const std::vector<std::vector<CellState>> &getStateGrid() const { return stateGrid; }
    // gScores for heatmap; infinity if unreachable
    const std::vector<std::vector<float>> &getGScoreGrid() const { return gScore; }

private:
    Grid &g;
    Coord start;
    Coord goal;

    std::vector<std::vector<CellState>> stateGrid;
    std::vector<std::vector<bool>> closed;
    std::vector<std::vector<float>> gScore;
    std::vector<std::vector<float>> fScore;
    std::vector<std::vector<Coord>> cameFrom;

    struct PQNode { float f; int id; int x; int y; };
    struct Compare {
        bool operator()(const PQNode &a, const PQNode &b) const {
            if (a.f == b.f) return a.id > b.id;
            return a.f > b.f;
        }
    };
    std::priority_queue<PQNode, std::vector<PQNode>, Compare> openPQ;
    int pushCounter = 0;

    bool done = false;
    bool solved = false;
    std::vector<Coord> solution;
    Coord current; // last node popped (for visualization)

    float heuristic(int x, int y) const;
    void reconstruct_path(int x, int y);
};

#endif // ASTAR_SOLVER_H
