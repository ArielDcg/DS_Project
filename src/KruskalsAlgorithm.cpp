#include "MazeAlgorithm.h"
#include <vector>
#include <queue>
#include <random>
#include <ctime>

struct KruskalsAlgorithm : public MazeAlgorithm {
    Grid &g;
    struct Edge { int x, y; int dir; int w; };
    struct EdgeLess { bool operator()(const Edge &a, const Edge &b) const { return a.w > b.w; } };
    std::priority_queue<Edge, std::vector<Edge>, EdgeLess> pq;

    std::vector<int> parent;
    std::vector<int> rnk;
    bool done = false;
    Coord lastCarved;

    KruskalsAlgorithm(Grid &grid) : g(grid) {
        const int W = g.width(), H = g.height();
        const int N = W * H;
        parent.resize(N); rnk.assign(N, 0);
        for (int i = 0; i < N; ++i) parent[i] = i;

        std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> dist(1, 1000000);

        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                if (x + 1 < W) pq.push(Edge{x, y, 2, dist(rng)});
                if (y + 1 < H) pq.push(Edge{x, y, 3, dist(rng)});
            }
        }
    }

    int idx(int x, int y) const { return y * g.width() + x; }

    int find(int a) {
        if (parent[a] != a) parent[a] = find(parent[a]);
        return parent[a];
    }

    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (rnk[a] < rnk[b]) std::swap(a, b);
        parent[b] = a;
        if (rnk[a] == rnk[b]) rnk[a]++;
    }

    bool step() override {
        if (done) return true;
        if (pq.empty()) { done = true; return true; }

        Edge e = pq.top(); pq.pop();
        int x = e.x, y = e.y;
        int nx = x, ny = y;
        if (e.dir == 2) nx = x + 1; else if (e.dir == 3) ny = y + 1;

        int a = idx(x, y);
        int b = idx(nx, ny);

        if (find(a) != find(b)) {
            g.removeWall(y, x, static_cast<short>(e.dir));
            unite(a, b);
            g.at(x, y).visited = true;
            g.at(nx, ny).visited = true;
            lastCarved = Coord(nx, ny);
            return false;
        }

        if (pq.empty()) done = true;
        return done;
    }

    bool finished() const override { return done; }
    bool getCurrent(Coord &out) const override {
        if (done) return false;
        out = lastCarved;
        return true;
    }
};
