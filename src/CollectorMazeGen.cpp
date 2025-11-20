// Algoritmos de generación modificados para modo coleccionista
// - Empiezan desde centro
// - Colocan tesoros al 33%, 66%, 90% de progreso

#include "MazeAlgorithm.h"
#include "ChallengeSystem.h"
#include <stack>
#include <vector>
#include <queue>
#include <random>
#include <cstdlib>
#include <ctime>

Coord getRandomCorner(const Grid& g, const Coord& avoid) {
    std::vector<Coord> corners = {
        Coord(0, 0),
        Coord(g.width() - 1, 0),
        Coord(0, g.height() - 1),
        Coord(g.width() - 1, g.height() - 1)
    };
    
    // Filtrar esquina a evitar
    std::vector<Coord> valid;
    for (const auto& c : corners) {
        if (!(c.x == avoid.x && c.y == avoid.y)) {
            valid.push_back(c);
        }
    }
    
    if (valid.empty()) return corners[0];
    return valid[std::rand() % valid.size()];
}

struct DFSCollectorAlgorithm : public MazeAlgorithm {
    Grid &g;
    ChallengeSystem *challenges;
    std::stack<Coord> path;
    bool done = false;
    int visitedCount = 0;
    int totalCells;
    bool treasure1Placed = false;
    bool treasure2Placed = false;
    bool treasure3Placed = false;
    
    DFSCollectorAlgorithm(Grid &grid, ChallengeSystem *cs = nullptr) 
        : g(grid), challenges(cs), totalCells(grid.width() * grid.height()) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        // Empezar desde centro
        Coord start(g.width() / 2, g.height() / 2);
        path.push(start);
        g.at(start.x, start.y).visited = true;
        visitedCount = 1;
    }

    bool step() override {
        if (done) return true;
        if (path.empty()) { done = true; return true; }

        Coord cur = path.top();
        int x = cur.x, y = cur.y;
        
        // Colocar tesoros según progreso
        if (challenges) {
            float progress = static_cast<float>(visitedCount) / totalCells;
            
            if (!treasure1Placed && progress >= 0.33f) {
                challenges->placeTreasureAt(cur);
                treasure1Placed = true;
            } else if (!treasure2Placed && progress >= 0.66f) {
                challenges->placeTreasureAt(cur);
                treasure2Placed = true;
            } else if (!treasure3Placed && progress >= 0.90f) {
                challenges->placeTreasureAt(cur);
                treasure3Placed = true;
            }
        }
        
        int dir = g.pickRandomNeighborDir(x, y);
        if (dir == -1) {
            path.pop();
        } else {
            int nx = x, ny = y;
            if (dir == 0) ny = y - 1;
            else if (dir == 1) nx = x - 1;
            else if (dir == 2) nx = x + 1;
            else if (dir == 3) ny = y + 1;

            g.removeWall(y, x, static_cast<short>(dir));
            g.at(nx, ny).visited = true;
            visitedCount++;
            path.push(Coord(nx, ny));
        }

        if (path.empty()) done = true;
        return done;
    }

    bool finished() const override { return done; }
    bool getCurrent(Coord &out) const override { 
        if (path.empty()) return false; 
        out = path.top(); 
        return true; 
    }
};

struct PrimsCollectorAlgorithm : public MazeAlgorithm {
    Grid &g;
    ChallengeSystem *challenges;
    struct Frontier { int cx, cy, dir; };
    std::vector<Frontier> frontier;
    bool done = false;
    Coord lastCarved;
    int visitedCount = 0;
    int totalCells;
    bool treasure1Placed = false;
    bool treasure2Placed = false;
    bool treasure3Placed = false;

    PrimsCollectorAlgorithm(Grid &grid, ChallengeSystem *cs = nullptr) 
        : g(grid), challenges(cs), totalCells(grid.width() * grid.height()) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        int sx = g.width() / 2;
        int sy = g.height() / 2;
        g.at(sx, sy).visited = true;
        visitedCount = 1;
        addFrontierFrom(sx, sy);
        lastCarved = Coord(sx, sy);
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

            g.removeWall(f.cy, f.cx, static_cast<short>(f.dir));
            g.at(nx, ny).visited = true;
            visitedCount++;
            addFrontierFrom(nx, ny);
            lastCarved = Coord(nx, ny);
            
            // Colocar tesoros
            if (challenges) {
                float progress = static_cast<float>(visitedCount) / totalCells;
                
                if (!treasure1Placed && progress >= 0.33f) {
                    challenges->placeTreasureAt(lastCarved);
                    treasure1Placed = true;
                } else if (!treasure2Placed && progress >= 0.66f) {
                    challenges->placeTreasureAt(lastCarved);
                    treasure2Placed = true;
                } else if (!treasure3Placed && progress >= 0.90f) {
                    challenges->placeTreasureAt(lastCarved);
                    treasure3Placed = true;
                }
            }

            frontier[idx] = frontier.back();
            frontier.pop_back();

            if (frontier.empty()) done = true;
            return false;
        }

        done = true;
        return true;
    }

    bool finished() const override { return done; }
    bool getCurrent(Coord &out) const override { 
        if (done) return false; 
        out = lastCarved; 
        return true; 
    }
};

struct HuntAndKillCollectorAlgorithm : public MazeAlgorithm {
    Grid &g;
    ChallengeSystem *challenges;
    int cx = 0, cy = 0;
    bool done = false;
    bool hunting = false;
    int visitedCount = 0;
    int totalCells;
    bool treasure1Placed = false;
    bool treasure2Placed = false;
    bool treasure3Placed = false;

    HuntAndKillCollectorAlgorithm(Grid &grid, ChallengeSystem *cs = nullptr)
        : g(grid), challenges(cs), totalCells(grid.width() * grid.height()) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        cx = g.width() / 2;
        cy = g.height() / 2;
        g.at(cx, cy).visited = true;
        visitedCount = 1;
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

    bool step() override {
        if (done) return true;

        // Kill phase
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
                visitedCount++;
                cx = nx; cy = ny;
                
                // Colocar tesoros
                if (challenges) {
                    float progress = static_cast<float>(visitedCount) / totalCells;
                    Coord current(cx, cy);
                    
                    if (!treasure1Placed && progress >= 0.33f) {
                        challenges->placeTreasureAt(current);
                        treasure1Placed = true;
                    } else if (!treasure2Placed && progress >= 0.66f) {
                        challenges->placeTreasureAt(current);
                        treasure2Placed = true;
                    } else if (!treasure3Placed && progress >= 0.90f) {
                        challenges->placeTreasureAt(current);
                        treasure3Placed = true;
                    }
                }
                
                return false;
            } else {
                hunting = true;
                return false;
            }
        }

        // Hunt phase
        for (int y = 0; y < g.height(); ++y) {
            for (int x = 0; x < g.width(); ++x) {
                if (!g.at(x, y).visited) {
                    int vdir = pickRandomVisitedNeighborDir(x, y);
                    if (vdir != -1) {
                        g.removeWall(y, x, static_cast<short>(vdir));
                        g.at(x, y).visited = true;
                        visitedCount++;
                        cx = x; cy = y;
                        hunting = false;
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
        if (done) return false;
        out = Coord(cx, cy);
        return true;
    }
};

struct KruskalsCollectorAlgorithm : public MazeAlgorithm {
    Grid &g;
    ChallengeSystem *challenges;
    struct Edge { int x, y; int dir; int w; };
    struct EdgeLess { bool operator()(const Edge &a, const Edge &b) const { return a.w > b.w; } };
    std::priority_queue<Edge, std::vector<Edge>, EdgeLess> pq;

    std::vector<int> parent;
    std::vector<int> rnk;
    bool done = false;
    Coord lastCarved;
    int visitedCount = 0;
    int totalCells;
    bool treasure1Placed = false;
    bool treasure2Placed = false;
    bool treasure3Placed = false;

    KruskalsCollectorAlgorithm(Grid &grid, ChallengeSystem *cs = nullptr)
        : g(grid), challenges(cs), totalCells(grid.width() * grid.height()) {
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
        
        lastCarved = Coord(W/2, H/2);
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
            visitedCount += 2;
            lastCarved = Coord(nx, ny);
            
            // Colocar tesoros
            if (challenges) {
                float progress = static_cast<float>(visitedCount) / totalCells;
                
                if (!treasure1Placed && progress >= 0.33f) {
                    challenges->placeTreasureAt(lastCarved);
                    treasure1Placed = true;
                } else if (!treasure2Placed && progress >= 0.66f) {
                    challenges->placeTreasureAt(lastCarved);
                    treasure2Placed = true;
                } else if (!treasure3Placed && progress >= 0.90f) {
                    challenges->placeTreasureAt(lastCarved);
                    treasure3Placed = true;
                }
            }
            
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