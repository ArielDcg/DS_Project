#include "MazeAlgorithm.h"
#include <vector>
#include <cstdlib>
#include <ctime>

// Hunt-and-Kill maze generator
// 1) Kill phase: random walk until trapped (no unvisited neighbors).
// 2) Hunt phase: scan grid; pick an unvisited cell with at least one visited neighbor,
//    carve to a random visited neighbor, and resume Kill from there.
// Repeat until no unvisited cells remain.
struct HuntAndKillAlgorithm : public MazeAlgorithm {
    Grid &g;
    int cx = 0, cy = 0;
    bool done = false;
    bool hasCurrent = false;

    HuntAndKillAlgorithm(Grid &grid) : g(grid) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        cx = std::rand() % g.width();
        cy = std::rand() % g.height();
        g.at(cx, cy).visited = true;
        hasCurrent = true;
    }

    // returns a random direction toward a VISITED neighbor of (x,y) or -1 if none
    int pickRandomVisitedNeighborDir(int x, int y) const {
        std::vector<int> dirs;
        if (y > 0 && g.at(x, y - 1).visited) dirs.push_back(0);                 // Up
        if (x > 0 && g.at(x - 1, y).visited) dirs.push_back(1);                 // Left
        if (x < g.width() - 1 && g.at(x + 1, y).visited) dirs.push_back(2);     // Right
        if (y < g.height() - 1 && g.at(x, y + 1).visited) dirs.push_back(3);    // Down
        if (dirs.empty()) return -1;
        return dirs[std::rand() % dirs.size()];
    }

    bool step() override {
        if (done) return true;

        // --- Kill phase: random walk to any unvisited neighbor ---
        int dir = g.pickRandomNeighborDir(cx, cy);
        if (dir != -1) {
            int nx = cx, ny = cy;
            if (dir == 0) ny = cy - 1;          // up
            else if (dir == 1) nx = cx - 1;     // left
            else if (dir == 2) nx = cx + 1;     // right
            else if (dir == 3) ny = cy + 1;     // down

            // knock down wall between (cy,cx) and (ny,nx)
            g.removeWall(cy, cx, static_cast<short>(dir));
            g.at(nx, ny).visited = true;
            cx = nx; cy = ny;
            return false; // one carve per step for smooth animation
        }

        // --- Hunt phase: scan for an unvisited cell with a visited neighbor ---
        for (int y = 0; y < g.height(); ++y) {
            for (int x = 0; x < g.width(); ++x) {
                if (!g.at(x, y).visited) {
                    int vdir = pickRandomVisitedNeighborDir(x, y);
                    if (vdir != -1) {
                        g.removeWall(y, x, static_cast<short>(vdir));
                        g.at(x, y).visited = true;
                        cx = x; cy = y;
                        return false;
                    }
                }
            }
        }

        // No more unvisited cells with visited neighbors -> finished
        done = true;
        return true;
    }

    bool finished() const override { return done; }
    bool getCurrent(Coord &out) const override {
        if (!hasCurrent || done) return false;
        out = Coord(cx, cy);
        return true;
    }
};
