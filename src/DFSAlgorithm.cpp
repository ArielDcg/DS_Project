#include "MazeAlgorithm.h"
#include <stack>
#include <cstdlib>
#include <ctime>

struct DFSAlgorithm : public MazeAlgorithm {
    Grid &g;
    std::stack<Coord> path;
    bool done = false;

    DFSAlgorithm(Grid &grid) : g(grid) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        Coord start(std::rand() % g.width(), std::rand() % g.height());
        path.push(start);
        g.at(start.x, start.y).visited = true;
    }

    bool step() override {
        if (done) return true;
        if (path.empty()) { done = true; return true; }

        Coord cur = path.top();
        int x = cur.x, y = cur.y;
        int dir = g.pickRandomNeighborDir(x, y);
        if (dir == -1) {
            path.pop();
        } else {
            int nx = x, ny = y;
            if (dir == 0) ny = y - 1;      // up
            else if (dir == 1) nx = x - 1; // left
            else if (dir == 2) nx = x + 1; // right
            else if (dir == 3) ny = y + 1; // down

            // remove wall between (y,x) and (ny,nx)
            g.removeWall(y, x, static_cast<short>(dir));
            g.at(nx, ny).visited = true;
            path.push(Coord(nx, ny));
        }

        if (path.empty()) done = true;
        return done;
    }

    bool finished() const override { return done; }
    bool getCurrent(Coord &out) const override { if (path.empty()) return false; out = path.top(); return true; }
};