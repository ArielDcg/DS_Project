#include "OriginShiftMaze.h"
#include <queue>
#include <algorithm>

OriginShiftMaze::OriginShiftMaze(Grid& grid, int numRoots)
    : g(grid),
      numRoots(numRoots),
      parentMap(grid.width(), std::vector<int>(grid.height(), -1)),
      rng(std::random_device{}())
{
    // Distribuir raíces en diferentes partes del mapa
    roots.clear();
    
    // Raíz 1: Centro
    roots.push_back(Coord(grid.width() / 2, grid.height() / 2));
    
    if (numRoots >= 2) {
        // Raíz 2: Esquina superior izquierda
        roots.push_back(Coord(grid.width() / 4, grid.height() / 4));
    }
    
    if (numRoots >= 3) {
        // Raíz 3: Esquina inferior derecha
        roots.push_back(Coord(3 * grid.width() / 4, 3 * grid.height() / 4));
    }
    
    if (numRoots >= 4) {
        // Raíz 4: Esquina superior derecha
        roots.push_back(Coord(3 * grid.width() / 4, grid.height() / 4));
    }
}

int OriginShiftMaze::oppositeDir(int dir) const {
    switch (dir) {
        case 0: return 3; // Up <-> Down
        case 3: return 0;
        case 1: return 2; // Left <-> Right
        case 2: return 1;
        default: return -1;
    }
}

std::vector<std::pair<Coord, int>> OriginShiftMaze::getNeighbors(const Coord& c) const {
    std::vector<std::pair<Coord, int>> neighbors;
    
    if (c.y > 0) neighbors.push_back({Coord(c.x, c.y - 1), 0});
    if (c.x > 0) neighbors.push_back({Coord(c.x - 1, c.y), 1});
    if (c.x < g.width() - 1) neighbors.push_back({Coord(c.x + 1, c.y), 2});
    if (c.y < g.height() - 1) neighbors.push_back({Coord(c.x, c.y + 1), 3});
    
    return neighbors;
}

void OriginShiftMaze::initializeFromMaze() {
    // Usar BFS desde la primera raíz para construir el árbol dirigido inicial
    std::vector<std::vector<bool>> visited(g.width(), std::vector<bool>(g.height(), false));
    std::queue<Coord> q;
    
    // Marcar todas las raíces como visitadas y sin padre
    for (const Coord& root : roots) {
        q.push(root);
        visited[root.x][root.y] = true;
        parentMap[root.x][root.y] = -1;
    }
    
    while (!q.empty()) {
        Coord cur = q.front();
        q.pop();
        
        auto neighbors = getNeighbors(cur);
        for (auto& [neighbor, dir] : neighbors) {
            if (visited[neighbor.x][neighbor.y]) continue;
            if (g.at(cur.x, cur.y).walls[dir]) continue;
            
            visited[neighbor.x][neighbor.y] = true;
            parentMap[neighbor.x][neighbor.y] = oppositeDir(dir);
            q.push(neighbor);
        }
    }
}

void OriginShiftMaze::updateSingleRoot(int rootIndex) {
    if (rootIndex < 0 || rootIndex >= static_cast<int>(roots.size())) return;
    
    Coord& root = roots[rootIndex];
    auto neighbors = getNeighbors(root);
    if (neighbors.empty()) return;
    
    std::uniform_int_distribution<int> dist(0, neighbors.size() - 1);
    auto [newRoot, dirToNewRoot] = neighbors[dist(rng)];
    
    // La antigua raíz ahora apunta hacia la nueva raíz
    parentMap[root.x][root.y] = dirToNewRoot;
    
    // La nueva raíz ya no apunta a nadie
    parentMap[newRoot.x][newRoot.y] = -1;
    
    root = newRoot;
}

void OriginShiftMaze::update() {
    // Actualizar TODAS las raíces simultáneamente
    for (int i = 0; i < static_cast<int>(roots.size()); ++i) {
        updateSingleRoot(i);
    }
}

void OriginShiftMaze::applyToGrid() {
    // Resetear todas las paredes
    for (int x = 0; x < g.width(); ++x) {
        for (int y = 0; y < g.height(); ++y) {
            for (int d = 0; d < 4; ++d) {
                g.at(x, y).walls[d] = true;
            }
        }
    }
    
    // Quitar paredes según parentMap
    for (int x = 0; x < g.width(); ++x) {
        for (int y = 0; y < g.height(); ++y) {
            int parentDir = parentMap[x][y];
            if (parentDir == -1) continue;
            
            g.at(x, y).walls[parentDir] = false;
            
            int nx = x, ny = y;
            if (parentDir == 0) ny -= 1;
            else if (parentDir == 1) nx -= 1;
            else if (parentDir == 2) nx += 1;
            else if (parentDir == 3) ny += 1;
            
            if (nx >= 0 && ny >= 0 && nx < g.width() && ny < g.height()) {
                g.at(nx, ny).walls[oppositeDir(parentDir)] = false;
            }
        }
    }
}

int OriginShiftMaze::getEdgeCount() const {
    int count = 0;
    for (int x = 0; x < g.width(); ++x) {
        for (int y = 0; y < g.height(); ++y) {
            // Contar aristas hacia la derecha y hacia abajo para evitar duplicados
            if (!g.at(x, y).walls[2] && x < g.width() - 1) count++;
            if (!g.at(x, y).walls[3] && y < g.height() - 1) count++;
        }
    }
    return count;
}
