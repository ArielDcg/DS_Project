#include "MazeSolver.h"
#include <algorithm>

MazeSolver::MazeSolver(Grid &grid, Coord s, Coord g)
    : g(grid), start(s), goal(g), visited(grid.width(), std::vector<bool>(grid.height(), false)),
      cameFrom(grid.width(), std::vector<Coord>(grid.height(), Coord(-1, -1))) {
    path.push(start);
    visited[start.x][start.y] = true;
}

bool MazeSolver::step() {
    if (solved || path.empty()) return true;

    Coord cur = path.top();

    if (cur.x == goal.x && cur.y == goal.y) {
        // reconstruir el camino
        Coord step = goal;
        while (!(step.x == -1 && step.y == -1)) {
            solution.push_back(step);
            step = cameFrom[step.x][step.y];
        }
        std::reverse(solution.begin(), solution.end());
        solved = true;
        return true;
    }

    for (int dir = 0; dir < 4; ++dir) {
        if (!g.at(cur.x, cur.y).walls[dir]) {
            int nx = cur.x, ny = cur.y;
            if (dir == 0) ny -= 1;
            else if (dir == 1) nx -= 1;
            else if (dir == 2) nx += 1;
            else if (dir == 3) ny += 1;

            if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
            if (visited[nx][ny]) continue;

            visited[nx][ny] = true;
            cameFrom[nx][ny] = cur;
            path.push(Coord(nx, ny));
            return false;
        }
    }

    path.pop(); // retrocede si no hay salida
    return false;
}

bool MazeSolver::finished() const {
    return solved;
}

bool MazeSolver::getCurrent(Coord &out) const {
    if (path.empty()) return false;
    out = path.top();
    return true;
}

const std::vector<Coord> &MazeSolver::getSolution() const {
    return solution;
}


