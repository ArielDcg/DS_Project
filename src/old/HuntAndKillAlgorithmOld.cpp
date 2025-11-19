#include "MazeAlgorithm.h"
#include <vector>
#include <cstdlib>
#include <ctime>

// Hunt-and-Kill Maze Generation Algorithm
// ----------------------------------------
// 1. Start from a random cell.
// 2. Randomly walk to any unvisited neighbor until trapped.
// 3. Hunt: find an unvisited cell with a visited neighbor,
//    connect them, and resume random walk from there.
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

    // returns a random unvisited neighbor direction, or -1 if none
    int pickRandomUnvisitedNeighborDir(int x, int y) const {
        std::vector<int> dirs;
        if (y > 0 && !g.at(x, y - 1).visited) dirs.push_back(0);                // Up
        if (x > 0 && !g.at(x - 1, y).visited) dirs.push_back(1);                // Left
        if (x < g.width() - 1 && !g.at(x + 1, y).visited) dirs.push_back(2);    // Right
        if (y < g.height() - 1 && !g.at(x, y + 1).visited) dirs.push_back(3);   // Down
        if (dirs.empty()) return -1;
        return dirs[std::rand() % dirs.size()];
    }

    // returns a random direction toward a *visited* neighbor, or -1 if none
    int pickRandomVisitedNeighborDir(int x, int y) const {
        std::vector<int> dirs;
        if (y > 0 && g.at(x, y - 1).visited) dirs.push_back(0);                // Up
        if (x > 0 && g.at(x - 1, y).visited) dirs.push_back(1);                // Left
        if (x < g.width() - 1 && g.at(x + 1, y).visited) dirs.push_back(2);    // Right
        if (y < g.height() - 1 && g.at(x, y + 1).visited) dirs.push_back(3);   // Down
        if (dirs.empty()) return -1;
        return dirs[std::rand() % dirs.size()];
    }

    bool step() override {
        if (done) return true;

        // ---- Kill phase ----
        int dir = pickRandomUnvisitedNeighborDir(cx, cy);
        if (dir != -1) {
            int nx = cx, ny = cy;
            if (dir == 0) ny -= 1;      // Up
            else if (dir == 1) nx -= 1; // Left
            else if (dir == 2) nx += 1; // Right
            else if (dir == 3) ny += 1; // Down

            g.removeWall(cy, cx, static_cast<short>(dir)); // Correct: row=cy, col=cx
            g.at(nx, ny).visited = true;
            cx = nx;
            cy = ny;
            return false; // smooth animation
        }

        // ---- Hunt phase ----
        for (int y = 0; y < g.height(); ++y) {
            for (int x = 0; x < g.width(); ++x) {
                if (!g.at(x, y).visited) {
                    int vdir = pickRandomVisitedNeighborDir(x, y);
                    if (vdir != -1) {
                        // Connect this unvisited cell to the visited neighbor
                        g.removeWall(y, x, static_cast<short>(vdir));
                        g.at(x, y).visited = true;
                        cx = x;
                        cy = y;
                        return false;
                    }
                }
            }
        }

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
