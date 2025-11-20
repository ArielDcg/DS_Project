#include "ExplorationHeatmap.h"
#include <numeric>

ExplorationHeatmap::ExplorationHeatmap(int w, int h) 
    : width(w), height(h) {
    // Reservar espacio para 400 celdas 
    visitCounts.reserve(500);
}

void ExplorationHeatmap::recordVisit(int x, int y) {

    Coord key(x, y);
    visitCounts[key]++;
    
    // Invalidar cache
    cachedMaxVisits = -1;
}

int ExplorationHeatmap::getVisitCount(int x, int y) const {

    Coord key(x, y);
    auto it = visitCounts.find(key);
    
    // Si no existe, retornar 0 (valor por defecto)
    return (it != visitCounts.end()) ? it->second : 0;
}

int ExplorationHeatmap::getMaxVisits() const {
    // Cache para evitar recalcular
    if (cachedMaxVisits >= 0) return cachedMaxVisits;
    
    if (visitCounts.empty()) {
        cachedMaxVisits = 0;
        return 0;
    }

    int maxV = 0;
    for (const auto& pair : visitCounts) {
        maxV = std::max(maxV, pair.second);
    }
    
    cachedMaxVisits = maxV;
    return maxV;
}

int ExplorationHeatmap::getTotalCellsVisited() const {
    return static_cast<int>(visitCounts.size());
}

void ExplorationHeatmap::clear() {
    visitCounts.clear();
    cachedMaxVisits = -1;
}

std::vector<std::vector<float>> ExplorationHeatmap::toNormalizedGrid() const {
    // Grid denso para visualización
    std::vector<std::vector<float>> grid(
        width, std::vector<float>(height, 0.0f)
    );
    
    int maxV = getMaxVisits();
    if (maxV == 0) return grid;
    
    for (const auto& pair : visitCounts) {
        const Coord& coord = pair.first;
        int visits = pair.second;
        
        // Normalizar [0.0, 1.0]
        grid[coord.x][coord.y] = static_cast<float>(visits) / maxV;
    }
    
    return grid;
}

ExplorationHeatmap::HeatmapStats ExplorationHeatmap::getStats() const {
    HeatmapStats stats;
    
    if (visitCounts.empty()) return stats;
    
    stats.uniqueCells = static_cast<int>(visitCounts.size());
    stats.maxVisits = getMaxVisits();
    
    // Calcular total y promedio
    int totalVisits = 0;
    for (const auto& pair : visitCounts) {
        totalVisits += pair.second;
    }
    stats.totalVisits = totalVisits;
    stats.avgVisits = static_cast<float>(totalVisits) / stats.uniqueCells;
    
    // Densidad (% del grid visitado)
    int totalCells = width * height;
    stats.densityPercent = (static_cast<float>(stats.uniqueCells) / totalCells) * 100.0f;
    
    return stats;
}