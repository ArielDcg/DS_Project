#include "GreedySolver.h"
#include <cmath>
#include <algorithm>
#include <limits>

GreedySolver::GreedySolver(Grid &grid, Coord s, Coord g_)
    : g(grid), start(s), goal(g_),
      stateGrid(grid.width(), std::vector<CellState>(grid.height(), UNKNOWN)),
      closed(grid.width(), std::vector<bool>(grid.height(), false)),
      gScore(grid.width(), std::vector<float>(grid.height(), std::numeric_limits<float>::infinity())),
      cameFrom(grid.width(), std::vector<Coord>(grid.height(), Coord(-1,-1))),
      current(s) {

    gScore[start.x][start.y] = 0.0f;
    float h = heuristic(start.x, start.y);
    openPQ.push({h, pushCounter++, start.x, start.y});
    stateGrid[start.x][start.y] = OPEN;
}

float GreedySolver::heuristic(int x, int y) const {
    // Manhattan
    return static_cast<float>(std::abs(goal.x - x) + std::abs(goal.y - y));
}

bool GreedySolver::step() {
    if (done) return true;
    if (openPQ.empty()) { done = true; solved = false; return true; }

    PQNode node;
    do {
        if (openPQ.empty()) { done = true; solved = false; return true; }
        node = openPQ.top();
        openPQ.pop();
    } while (closed[node.x][node.y]);

    int cx = node.x, cy = node.y;
    current = Coord(cx, cy);

    closed[cx][cy] = true;
    stateGrid[cx][cy] = CLOSED;

    if (cx == goal.x && cy == goal.y) {
        reconstruct_path(cx, cy);
        done = true;
        solved = true;
        return true;
    }

    for (int dir = 0; dir < 4; ++dir) {
        if (g.at(cx, cy).walls[dir]) continue;

        int nx = cx, ny = cy;
        if (dir == 0) ny -= 1;
        else if (dir == 1) nx -= 1;
        else if (dir == 2) nx += 1;
        else if (dir == 3) ny += 1;

        if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
        if (closed[nx][ny]) continue;

        float tentative_g = gScore[cx][cy] + 1.0f;

        if (tentative_g < gScore[nx][ny]) {
            cameFrom[nx][ny] = Coord(cx, cy);
            gScore[nx][ny] = tentative_g;
            float h = heuristic(nx, ny);
            openPQ.push({h, pushCounter++, nx, ny});
            stateGrid[nx][ny] = OPEN;
        }
    }

    return false;
}

void GreedySolver::reconstruct_path(int x, int y) {
    solution.clear();
    Coord cur(x, y);
    while (!(cur.x == -1 && cur.y == -1)) {
        solution.push_back(cur);
        cur = cameFrom[cur.x][cur.y];
    }
    std::reverse(solution.begin(), solution.end());
}

bool GreedySolver::finished() const { return done; }

bool GreedySolver::getCurrent(Coord &out) const {
    if (done) return false;
    out = current;
    return true;
}

const std::vector<Coord> &GreedySolver::getSolution() const { return solution; }
