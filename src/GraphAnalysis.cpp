#include "GraphAnalysis.h"
#include <limits>

GraphAnalysis::GraphAnalysis(const Grid& grid) : g(grid) {}

int GraphAnalysis::coordToId(int x, int y) const {
    return y * g.width() + x;
}

Coord GraphAnalysis::idToCoord(int id) const {
    return Coord(id % g.width(), id / g.width());
}

void GraphAnalysis::buildAdjacencyList() {
    adjList.clear();
    
    for (int x = 0; x < g.width(); ++x) {
        for (int y = 0; y < g.height(); ++y) {
            int nodeId = coordToId(x, y);
            const Cell& cell = g.at(x, y);
            
            // Agregar vecinos conectados (sin pared)
            // dir: 0=Up, 1=Left, 2=Right, 3=Down
            if (!cell.walls[0] && y > 0) {
                adjList[nodeId].push_back(coordToId(x, y - 1));
            }
            if (!cell.walls[1] && x > 0) {
                adjList[nodeId].push_back(coordToId(x - 1, y));
            }
            if (!cell.walls[2] && x < g.width() - 1) {
                adjList[nodeId].push_back(coordToId(x + 1, y));
            }
            if (!cell.walls[3] && y < g.height() - 1) {
                adjList[nodeId].push_back(coordToId(x, y + 1));
            }
        }
    }
}

std::pair<int, int> GraphAnalysis::bfsFarthest(int startId) {
    std::unordered_map<int, int> dist;
    std::queue<int> q;
    
    q.push(startId);
    dist[startId] = 0;
    
    int farthestNode = startId;
    int maxDist = 0;
    
    while (!q.empty()) {
        int curr = q.front();
        q.pop();
        
        for (int neighbor : adjList[curr]) {
            if (dist.find(neighbor) == dist.end()) {
                dist[neighbor] = dist[curr] + 1;
                q.push(neighbor);
                
                if (dist[neighbor] > maxDist) {
                    maxDist = dist[neighbor];
                    farthestNode = neighbor;
                }
            }
        }
    }
    
    return {farthestNode, maxDist};
}

std::vector<Coord> GraphAnalysis::reconstructPath(int fromId, int toId) {
    // BFS para encontrar camino
    std::unordered_map<int, int> parent;
    std::queue<int> q;
    
    q.push(fromId);
    parent[fromId] = -1;
    
    while (!q.empty()) {
        int curr = q.front();
        q.pop();
        
        if (curr == toId) break;
        
        for (int neighbor : adjList[curr]) {
            if (parent.find(neighbor) == parent.end()) {
                parent[neighbor] = curr;
                q.push(neighbor);
            }
        }
    }
    
    // Reconstruir camino
    std::vector<Coord> path;
    int curr = toId;
    while (curr != -1) {
        path.push_back(idToCoord(curr));
        curr = parent[curr];
    }
    
    // Invertir para que vaya de from a to
    std::reverse(path.begin(), path.end());
    return path;
}

GraphStats GraphAnalysis::calculateDiameter() {
    GraphStats stats;
    stats.diameter = 0;
    
    if (adjList.empty()) {
        buildAdjacencyList();
    }
    
    // Doble BFS para encontrar diámetro
    // 1. BFS desde cualquier nodo -> nodo más lejano A
    int startNode = coordToId(0, 0);
    auto [nodeA, distA] = bfsFarthest(startNode);
    
    // 2. BFS desde A -> nodo más lejano B (este es el diámetro)
    auto [nodeB, diameter] = bfsFarthest(nodeA);
    
    stats.diameter = diameter;
    stats.diameterStart = idToCoord(nodeA);
    stats.diameterEnd = idToCoord(nodeB);
    stats.path = reconstructPath(nodeA, nodeB);
    
    return stats;
}
