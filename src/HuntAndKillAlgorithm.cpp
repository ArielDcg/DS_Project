#include "MazeAlgorithm.h"
#include <vector>
#include <cstdlib>
#include <ctime>

struct HuntAndKillAlgorithm : public MazeAlgorithm {
    Grid &g;
    int cx = 0, cy = 0;
    bool done = false;
    bool hunting = false;

    HuntAndKillAlgorithm(Grid &grid) : g(grid) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        cx = std::rand() % g.width();
        cy = std::rand() % g.height();
        g.at(cx, cy).visited = true;
    }

    int pickRandomUnvisitedNeighborDir(int x, int y) const {
        std::vector<int> dirs;
        if (y > 0 && !g.at(x, y - 1).visited) dirs.push_back(0);
        if (x > 0 && !g.at(x - 1, y).visited) dirs.push_back(1);
        if (x < g.width() - 1 && !g.at(x + 1, y).visited) dirs.push_back(2);
        if (y < g.height() - 1 && !g.at(x, y + 1).visited) dirs.push_back(3);
        if (dirs.empty()) return -1;
        return dirs[std::rand() % dirs.size()];
    }

    int pickRandomVisitedNeighborDir(int x, int y) const {
        std::vector<int> dirs;
        if (y > 0 && g.at(x, y - 1).visited) dirs.push_back(0);
        if (x > 0 && g.at(x - 1, y).visited) dirs.push_back(1);
        if (x < g.width() - 1 && g.at(x + 1, y).visited) dirs.push_back(2);
        if (y < g.height() - 1 && g.at(x, y + 1).visited) dirs.push_back(3);
        if (dirs.empty()) return -1;
        return dirs[std::rand() % dirs.size()];
    }

    bool allVisited() const {
        for (int y = 0; y < g.height(); ++y)
            for (int x = 0; x < g.width(); ++x)
                if (!g.at(x, y).visited)
                    return false;
        return true;
    }

    bool step() override {
        if (done) return true;

        // === Kill phase (random walk) ===
        if (!hunting) {
            int dir = pickRandomUnvisitedNeighborDir(cx, cy);
            if (dir != -1) {
                int nx = cx, ny = cy;
                if (dir == 0) ny--;
                else if (dir == 1) nx--;
                else if (dir == 2) nx++;
                else if (dir == 3) ny++;

                g.removeWall(cy, cx, static_cast<short>(dir));
                g.at(nx, ny).visited = true;
                cx = nx; cy = ny;
                return false;
            } else {
                // no unvisited neighbors, start hunting
                hunting = true;
                return false;
            }
        }

        // === Hunt phase (find next unvisited with visited neighbor) ===
        for (int y = 0; y < g.height(); ++y) {
            for (int x = 0; x < g.width(); ++x) {
                if (!g.at(x, y).visited) {
                    int vdir = pickRandomVisitedNeighborDir(x, y);
                    if (vdir != -1) {
                        g.removeWall(y, x, static_cast<short>(vdir));
                        g.at(x, y).visited = true;
                        cx = x; cy = y;
                        hunting = false;
                        return false;
                    }
                }
            }
        }

        // If no cell found — maze complete
        done = true;
        return true;
    }

    bool finished() const override { return done; }

    bool getCurrent(Coord &out) const override {
        if (done) return false;
        out = Coord(cx, cy);
        return true;
    }
};
