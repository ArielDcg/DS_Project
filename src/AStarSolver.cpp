#include "AStarSolver.h"
#include <cmath>
#include <algorithm>
#include <limits>

AStarSolver::AStarSolver(Grid &grid, Coord s, Coord g_)
    : g(grid), start(s), goal(g_),
      closed(grid.width(), std::vector<bool>(grid.height(), false)),
      gScore(grid.width(), std::vector<float>(grid.height(), std::numeric_limits<float>::infinity())),
      fScore(grid.width(), std::vector<float>(grid.height(), std::numeric_limits<float>::infinity())),
      cameFrom(grid.width(), std::vector<Coord>(grid.height(), Coord(-1,-1))),
      current(s) {

    gScore[start.x][start.y] = 0.0f;
    fScore[start.x][start.y] = heuristic(start.x, start.y);
    openPQ.push({fScore[start.x][start.y], pushCounter++, start.x, start.y});
}

float AStarSolver::heuristic(int x, int y) const {
    // Manhattan distance (4-neighbor grid)
    return static_cast<float>(std::abs(goal.x - x) + std::abs(goal.y - y));
}

bool AStarSolver::step() {
    if (done) return true;
    if (openPQ.empty()) {
        done = true;
        solved = false;
        return true;
    }

    // pop best node, skipping nodes already closed (outdated pushes)
    PQNode node;
    do {
        if (openPQ.empty()) { done = true; solved = false; return true; }
        node = openPQ.top();
        openPQ.pop();
    } while (closed[node.x][node.y]);

    int cx = node.x, cy = node.y;
    current = Coord(cx, cy);
    if (cx == goal.x && cy == goal.y) {
        reconstruct_path(cx, cy);
        done = true;
        solved = true;
        return true;
    }

    closed[cx][cy] = true;

    // neighbors: for each direction check wall and bounds
    for (int dir = 0; dir < 4; ++dir) {
        // skip if wall present
        if (g.at(cx, cy).walls[dir]) continue;

        int nx = cx, ny = cy;
        if (dir == 0) ny -= 1;
        else if (dir == 1) nx -= 1;
        else if (dir == 2) nx += 1;
        else if (dir == 3) ny += 1;

        if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
        if (closed[nx][ny]) continue;

        float tentative_g = gScore[cx][cy] + 1.0f; // uniform cost for grid edges

        if (tentative_g < gScore[nx][ny]) {
            cameFrom[nx][ny] = Coord(cx, cy);
            gScore[nx][ny] = tentative_g;
            fScore[nx][ny] = tentative_g + heuristic(nx, ny);
            openPQ.push({fScore[nx][ny], pushCounter++, nx, ny});
        }
    }

    // expanded one node this step
    return false;
}

void AStarSolver::reconstruct_path(int x, int y) {
    solution.clear();
    Coord cur(x, y);
    while (!(cur.x == -1 && cur.y == -1)) {
        solution.push_back(cur);
        cur = cameFrom[cur.x][cur.y];
    }
    std::reverse(solution.begin(), solution.end());
}

bool AStarSolver::finished() const {
    return done;
}

bool AStarSolver::getCurrent(Coord &out) const {
    if (done) return false;
    out = current;
    return true;
}

const std::vector<Coord> &AStarSolver::getSolution() const {
    return solution;
}
