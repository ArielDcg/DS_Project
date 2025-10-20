struct DFSSolver {
    Grid &g;
    std::stack<Coord> path;
    std::vector<std::vector<bool>> visited;
    Coord start, goal;
    bool solved = false;

    DFSSolver(Grid &grid, Coord s, Coord g) : g(grid), start(s), goal(g) {
        visited.resize(g.width(), std::vector<bool>(g.height(), false));
        path.push(start);
        visited[start.x][start.y] = true;
    }

    bool step() {
        if (solved || path.empty()) return true;

        Coord cur = path.top();
        if (cur == goal) {
            solved = true;
            return true;
        }

        for (int dir = 0; dir < 4; ++dir) {
            int nx = cur.x, ny = cur.y;
            if (!g.canMove(cur.x, cur.y, dir)) continue;

            if (dir == 0) ny -= 1;      // up
            else if (dir == 1) nx -= 1; // left
            else if (dir == 2) nx += 1; // right
            else if (dir == 3) ny += 1; // down

            if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
            if (visited[nx][ny]) continue;

            visited[nx][ny] = true;
            path.push(Coord(nx, ny));
            return false;
        }

        path.pop(); // backtrack
        return false;
    }

    bool finished() const { return solved; }
    bool getCurrent(Coord &out) const { if (path.empty()) return false; out = path.top(); return true; }
};
