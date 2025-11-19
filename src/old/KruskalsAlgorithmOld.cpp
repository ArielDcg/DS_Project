#include "MazeAlgorithm.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>

// Randomized Kruskal's Algorithm for Maze Generation
// Treat each cell as a node; list all possible walls (edges) between adjacent cells.
// Shuffle edges; for each edge, if it connects two different sets, remove the wall and union the sets.
// Finish when all cells are connected or no edges remain.
struct KruskalsAlgorithm : public MazeAlgorithm {
    Grid &g;
    struct Edge { int x, y; int dir; };
    std::vector<Edge> edges;
    std::vector<int> parent;
    std::vector<int> rankv;
    bool done = false;
    Coord last;

    KruskalsAlgorithm(Grid &grid) : g(grid) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        const int W = g.width(), H = g.height();
        parent.resize(W * H);
        rankv.assign(W * H, 0);
        std::iota(parent.begin(), parent.end(), 0);

        // list edges between each cell and its right & down neighbor (to avoid duplicates)
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                if (x + 1 < W) edges.push_back({x, y, 2});
                if (y + 1 < H) edges.push_back({x, y, 3});
            }
        }
        // modern shuffle
        std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::shuffle(edges.begin(), edges.end(), rng);
    }

    int idx(int x, int y) const { return y * g.width() + x; }

    int find(int a) {
        if (parent[a] != a) parent[a] = find(parent[a]);
        return parent[a];
    }

    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (rankv[a] < rankv[b]) std::swap(a, b);
        parent[b] = a;
        if (rankv[a] == rankv[b]) rankv[a]++;
    }

    bool step() override {
        if (done) return true;
        if (edges.empty()) { done = true; return true; }

        // process one edge per step for smooth animation
        Edge e = edges.back();
        edges.pop_back();

        int x = e.x, y = e.y;
        int nx = x, ny = y;
        if (e.dir == 2) nx = x + 1;        // right neighbor
        else if (e.dir == 3) ny = y + 1;   // down neighbor
        else if (e.dir == 1) nx = x - 1;   // left (not generated)
        else if (e.dir == 0) ny = y - 1;   // up   (not generated)

        int a = idx(x, y);
        int b = idx(nx, ny);

        if (find(a) != find(b)) {
            // carve: remove wall on (y,x) in direction e.dir
            g.removeWall(y, x, static_cast<short>(e.dir));
            unite(a, b);

            g.at(x, y).visited = true;
            g.at(nx, ny).visited = true;
            last = Coord(nx, ny);
            return false; // one carve per frame
        }

        // If edge was discarded (same set), continue next step immediately next frame
        if (edges.empty()) done = true;
        return done;
    }

    bool finished() const override { return done; }
    bool getCurrent(Coord &out) const override {
        if (done) return false;
        out = last;
        return true;
    }
};
