#include "ChallengeSystem.h"
#include <algorithm>

ChallengeSystem::ChallengeSystem(Grid& grid) : g(grid) {}

void ChallengeSystem::placeTreasureAt(const Coord& pos) {
    // No colocar en inicio (centro)
    int centerX = g.width() / 2;
    int centerY = g.height() / 2;
    if (pos.x == centerX && pos.y == centerY) return;
    
    // No colocar en esquinas (posibles metas)
    if ((pos.x == 0 || pos.x == g.width() - 1) && 
        (pos.y == 0 || pos.y == g.height() - 1)) return;
    
    // No duplicar
    if (elements.find(pos) != elements.end()) return;
    
    // ===================================
    // INSERCIÓN EN TABLA HASH: O(1)
    // ===================================
    elements[pos] = SpecialElement::TREASURE;
    treasurePositions.push_back(pos);
}

bool ChallengeSystem::hasTreasure(const Coord& pos) const {
    // ===================================
    // BÚSQUEDA EN TABLA HASH: O(1)
    // Crítico: llamado ~1,000 veces durante pathfinding
    // ===================================
    auto it = elements.find(pos);
    return (it != elements.end() && it->second == SpecialElement::TREASURE);
}

bool ChallengeSystem::collectTreasure(const Coord& pos) {
    auto it = elements.find(pos);
    
    if (it == elements.end() || it->second != SpecialElement::TREASURE) {
        return false;
    }
    
    // Marcar como recolectado (cambiar a NONE)
    it->second = SpecialElement::NONE;
    collectedCount++;
    return true;
}

const std::vector<Coord>& ChallengeSystem::getTreasurePositions() const {
    return treasurePositions;
}

bool ChallengeSystem::allTreasuresCollected() const {
    return collectedCount == static_cast<int>(treasurePositions.size());
}

int ChallengeSystem::remainingTreasures() const {
    return static_cast<int>(treasurePositions.size()) - collectedCount;
}

void ChallengeSystem::clear() {
    elements.clear();
    treasurePositions.clear();
    collectedCount = 0;
}