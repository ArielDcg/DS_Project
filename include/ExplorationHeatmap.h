#pragma once
#include "Grid.h"
#include "CoordHash.h" 
#include <unordered_map>
#include <vector>
#include <algorithm>


class ExplorationHeatmap {
public:
    ExplorationHeatmap(int width, int height);
    
    // Registrar visita a una celda
    void recordVisit(int x, int y);
    
    // Obtener número de visitas
    int getVisitCount(int x, int y) const;
    
    // Obtener máximo de visitas (para normalización)
    int getMaxVisits() const;
    
    // Obtener número total de celdas visitadas
    int getTotalCellsVisited() const;
    
    // Limpiar heatmap
    void clear();
    
    // Obtener grid normalizado [0.0, 1.0] para visualización
    std::vector<std::vector<float>> toNormalizedGrid() const;
    
    // Obtener estadísticas
    struct HeatmapStats {
        int totalVisits = 0;      // Suma de todas las visitas
        int uniqueCells = 0;       // Celdas únicas visitadas
        int maxVisits = 0;         // Máximo en una celda
        float avgVisits = 0.0f;    // Promedio de visitas
        float densityPercent = 0.0f; // % de grid visitado
    };
    HeatmapStats getStats() const;

private:
    int width, height;
    
    std::unordered_map<Coord, int, CoordHash> visitCounts;
    
    mutable int cachedMaxVisits = -1; 
};