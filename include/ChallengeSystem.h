#pragma once
#include "Grid.h"
#include "CoordHash.h" 
#include <unordered_map>
#include <vector>


enum class SpecialElement {
    TREASURE,
    NONE
};

// ===================================
// SISTEMA DE DESAFÍOS CON TABLA HASH
// ESTRUCTURA 1 de 3 (✅ Implementada)
// ===================================
class ChallengeSystem {
public:
    ChallengeSystem(Grid& grid);
    
    // Colocar tesoro en posición específica
    void placeTreasureAt(const Coord& pos);
    
    // Verificar si una coordenada tiene tesoro
    bool hasTreasure(const Coord& pos) const;
    
    // Recolectar tesoro (cambia estado a NONE)
    bool collectTreasure(const Coord& pos);
    
    // Obtener todas las posiciones de tesoros
    const std::vector<Coord>& getTreasurePositions() const;
    
    // Verificar si todos fueron recolectados
    bool allTreasuresCollected() const;
    
    // Número de tesoros restantes
    int remainingTreasures() const;
    
    // Limpiar todos los tesoros
    void clear();

    // Público para que CollectorSolver pueda acceder
    int collectedCount = 0;

private:
    Grid& g;
    
    // ===================================
    // TABLA HASH: Coord -> SpecialElement
    // Búsqueda O(1) promedio
    // Densidad: 0.27% (3/1120)
    // Justificación: Búsqueda crítica en pathfinding
    // ===================================
    std::unordered_map<Coord, SpecialElement, CoordHash> elements;
    
    // Lista para iterar fácilmente
    std::vector<Coord> treasurePositions;
};