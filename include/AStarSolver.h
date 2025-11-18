#ifndef ASTAR_SOLVER_H
#define ASTAR_SOLVER_H

#include "Grid.h"      // Grid.h defines Coord and Cell etc.
#include <vector>
#include <queue>       // priority_queue
#include <limits>

class AStarSolver {
public:
    AStarSolver(Grid &grid, Coord start, Coord goal);

    // advance one atomic step (expand one node or finish)
    // returns true if finished (either solved or impossible), false if more steps remain
    bool step();

    bool finished() const;
    bool getCurrent(Coord &out) const; // current expanded node for visualization
    const std::vector<Coord> &getSolution() const;

private:
    Grid &g;
    Coord start;
    Coord goal;

    std::vector<std::vector<bool>> closed;
    std::vector<std::vector<float>> gScore;
    std::vector<std::vector<float>> fScore;
    std::vector<std::vector<Coord>> cameFrom;

    // open set implemented as priority queue of (f, counter, x, y)
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
