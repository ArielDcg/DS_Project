struct MazeSolver {
    Grid &g;
    Coord start, goal;
    std::stack<Coord> path;
    std::vector<std::vector<bool>> visited;
    bool solved = false;

    MazeSolver(Grid &grid, Coord s, Coord g) : g(grid), start(s), goal(g) {
        visited.resize(g.width(), std::vector<bool>(g.height(), false));
        path.push(start);
        visited[start.x][start.y] = true;
    };

    bool step() {
        if (solved || path.empty()) return true;

        Coord cur = path.top();
        if (cur == goal) {
            solved = true;
            return true;
        };

        for (int dir = 0; dir < 4; ++dir) {
            int nx = cur.x, ny = cur.y;
            if (!g.canMove(cur.x, cur.y, dir)) continue;

            if (dir == 0) ny -= 1;
            else if (dir == 1) nx -= 1;
            else if (dir == 2) nx += 1;
            else if (dir == 3) ny += 1;

            if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
            if (visited[nx][ny]) continue;

            visited[nx][ny] = true;
            path.push(Coord(nx, ny));
            return false;
        };

        path.pop(); // retrocede si no hay salida
        return false;
    };

    bool finished() const { return solved; }
    bool getCurrent(Coord &out) const { if (path.empty()) return false; out = path.top(); return true; }
};

