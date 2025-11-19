#include "MazeAlgorithm.h"
#include <vector>
#include <cstdlib>
#include <ctime>

struct HuntAndKillAlgorithm : public MazeAlgorithm {
    Grid &g;
    bool done = false;
    int cx = 0, cy = 0;
    bool hasCurrent = false;

    struct CQ {
        std::vector<Coord> buf;
        int head = 0, tail = 0, cnt = 0;
        void init(int capacity) { buf.assign(capacity, Coord()); head = tail = cnt = 0; }
        bool empty() const { return cnt == 0; }
        bool full() const { return cnt == (int)buf.size(); }
        bool push(const Coord &c) {
            if (full()) return false;
            buf[tail] = c;
            tail = (tail + 1) % (int)buf.size();
            ++cnt;
            return true;
        }
        bool pop(Coord &out) {
            if (empty()) return false;
            out = buf[head];
            head = (head + 1) % (int)buf.size();
            --cnt;
            return true;
        }
    } huntQ;

    HuntAndKillAlgorithm(Grid &grid) : g(grid) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        cx = std::rand() % g.width();
        cy = std::rand() % g.height();
        g.at(cx, cy).visited = true;
        hasCurrent = true;

        huntQ.init(g.width() * g.height());
        for (int y = 0; y < g.height(); ++y)
            for (int x = 0; x < g.width(); ++x)
                huntQ.push(Coord(x, y));
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

    bool step() override {
        if (done) return true;

        int dir = g.pickRandomNeighborDir(cx, cy);
        if (dir != -1) {
            int nx = cx, ny = cy;
            if (dir == 0) ny = cy - 1;
            else if (dir == 1) nx = cx - 1;
            else if (dir == 2) nx = cx + 1;
            else if (dir == 3) ny = cy + 1;

            g.removeWall(cy, cx, static_cast<short>(dir));
            g.at(nx, ny).visited = true;
            cx = nx; cy = ny;
            return false;
        }

        Coord candidate;
        while (!huntQ.empty()) {
            huntQ.pop(candidate);
            int x = candidate.x, y = candidate.y;
            if (g.at(x, y).visited) continue;
            int vdir = pickRandomVisitedNeighborDir(x, y);
            if (vdir != -1) {
                g.removeWall(y, x, static_cast<short>(vdir));
                g.at(x, y).visited = true;
                cx = x; cy = y;
                hasCurrent = true;
                return false;
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
