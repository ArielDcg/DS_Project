#include "MazeAlgorithm.h"
#include <vector>
#include <cstdlib>
#include <ctime>

struct PrimsAlgorithm : public MazeAlgorithm {
    Grid &g;
    struct Frontier { int cx, cy, dir; };
    std::vector<Frontier> frontier;
    bool done = false;
    Coord lastCarved;

    PrimsAlgorithm(Grid &grid) : g(grid) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        int sx = std::rand() % g.width();
        int sy = std::rand() % g.height();
        g.at(sx, sy).visited = true;
        addFrontierFrom(sx, sy);
    }

    void addFrontierFrom(int cx, int cy) {
        if (cy > 0 && !g.at(cx, cy - 1).visited) frontier.push_back({cx, cy, 0});
        if (cy < g.height() - 1 && !g.at(cx, cy + 1).visited) frontier.push_back({cx, cy, 3});
        if (cx > 0 && !g.at(cx - 1, cy).visited) frontier.push_back({cx, cy, 1});
        if (cx < g.width() - 1 && !g.at(cx + 1, cy).visited) frontier.push_back({cx, cy, 2});
    }

    bool step() override {
        if (done) return true;
        while (!frontier.empty()) {
            int idx = std::rand() % frontier.size();
            Frontier f = frontier[idx];

            int nx = f.cx, ny = f.cy;
            if (f.dir == 0) ny = f.cy - 1;
            else if (f.dir == 1) nx = f.cx - 1;
            else if (f.dir == 2) nx = f.cx + 1;
            else if (f.dir == 3) ny = f.cy + 1;

            if (g.at(nx, ny).visited) {
                frontier[idx] = frontier.back();
                frontier.pop_back();
                continue;
            }

            // carve
            g.removeWall(f.cy, f.cx, static_cast<short>(f.dir));
            g.at(nx, ny).visited = true;
            addFrontierFrom(nx, ny);
            lastCarved = Coord(nx, ny);

            // remove used frontier
            frontier[idx] = frontier.back();
            frontier.pop_back();

            if (frontier.empty()) done = true;
            return false; // one carve per step
        }

        done = true;
        return true;
    }

    bool finished() const override { return done; }
    bool getCurrent(Coord &out) const override { if (done) return false; out = lastCarved; return true; }
};