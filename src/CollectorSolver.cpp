#include "CollectorSolver.h"
#include <cmath>
#include <algorithm>

CollectorSolver::CollectorSolver(Grid& grid, ChallengeSystem& chall, Coord s, Coord fg)
    : g(grid), challenges(chall), start(s), finalGoal(fg),
      stateGrid(grid.width(), std::vector<CellState>(grid.height(), UNKNOWN)),
      closed(grid.width(), std::vector<bool>(grid.height(), false)),
      gScore(grid.width(), std::vector<float>(grid.height(), std::numeric_limits<float>::infinity())),
      fScore(grid.width(), std::vector<float>(grid.height(), std::numeric_limits<float>::infinity())),
      cameFrom(grid.width(), std::vector<Coord>(grid.height(), Coord(-1,-1))),
      currentPos(s) {
    
    // ===================================
    // 🎯 GREEDY MEJORADO CORREGIDO
    // ===================================
    // Ordenar tesoros de MÁS ALEJADO a MÁS CERCANO de la meta
    // Así terminamos cerca de la meta al final
    objectives = greedyOrderTreasures(start, challenges.getTreasurePositions(), finalGoal);
    objectives.push_back(finalGoal);
    
    if (!objectives.empty()) {
        currentGoal = objectives[0];
        initializeSearch(start, currentGoal);
    } else {
        allDone = true;
    }
}

float CollectorSolver::manhattan(const Coord& a, const Coord& b) const {
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

std::vector<Coord> CollectorSolver::greedyOrderTreasures(
    Coord from, 
    const std::vector<Coord>& treasures, 
    Coord goal
) {
    std::vector<Coord> remaining = treasures;
    std::vector<Coord> ordered;
    Coord current = from;
    
    while (!remaining.empty()) {
        // ===================================
        // 🔧 CORRECCIÓN: Buscar MÁXIMO no mínimo
        // ===================================
        float bestScore = -std::numeric_limits<float>::infinity();  // ← Empezar en -∞
        int bestIdx = 0;
        
        // Para cada tesoro: calcular distancia total (actual→tesoro + tesoro→meta)
        for (size_t i = 0; i < remaining.size(); ++i) {
            float toTreasure = manhattan(current, remaining[i]);
            float treasureToGoal = manhattan(remaining[i], goal);
            float totalScore = toTreasure + treasureToGoal;
            
            // ===================================
            // ✅ BUSCAR MÁXIMO (tesoro más alejado)
            // ===================================
            if (totalScore > bestScore) {  // ← CORREGIDO: > en lugar de <
                bestScore = totalScore;
                bestIdx = static_cast<int>(i);
            }
        }
        
        // Agregar el tesoro MÁS ALEJADO y actualizar posición
        ordered.push_back(remaining[bestIdx]);
        current = remaining[bestIdx];
        remaining.erase(remaining.begin() + bestIdx);
    }
    
    return ordered;
}

void CollectorSolver::initializeSearch(Coord from, Coord to) {
    // ===================================
    // 🧹 LIMPIEZA COMPLETA (Opción A)
    // ===================================
    // Resetear TODAS las estructuras entre objetivos
    closed.assign(g.width(), std::vector<bool>(g.height(), false));
    gScore.assign(g.width(), std::vector<float>(g.height(), std::numeric_limits<float>::infinity()));
    fScore.assign(g.width(), std::vector<float>(g.height(), std::numeric_limits<float>::infinity()));
    cameFrom.assign(g.width(), std::vector<Coord>(g.height(), Coord(-1,-1)));
    stateGrid.assign(g.width(), std::vector<CellState>(g.height(), UNKNOWN));
    
    // Limpiar priority queue
    while (!openPQ.empty()) openPQ.pop();
    pushCounter = 0;
    
    // Inicializar A* desde posición actual
    currentPos = from;
    currentGoal = to;
    gScore[from.x][from.y] = 0.0f;
    fScore[from.x][from.y] = heuristic(from.x, from.y, to);
    openPQ.push({fScore[from.x][from.y], pushCounter++, from.x, from.y});
    stateGrid[from.x][from.y] = OPEN;
    
    segmentDone = false;
    currentSegmentPath.clear();
}

float CollectorSolver::heuristic(int x, int y, const Coord& goal) const {
    // Manhattan distance (admisible para grid 4-direccional)
    return static_cast<float>(std::abs(goal.x - x) + std::abs(goal.y - y));
}

bool CollectorSolver::step() {
    if (allDone) return true;
    
    if (segmentDone) {
        // Pasar al siguiente objetivo
        currentObjectiveIndex++;
        
        if (currentObjectiveIndex >= static_cast<int>(objectives.size())) {
            allDone = true;
            return true;
        }
        
        Coord nextGoal = objectives[currentObjectiveIndex];
        initializeSearch(currentPos, nextGoal);
        return false;
    }
    
    return stepAStar();
}

bool CollectorSolver::stepAStar() {
    if (openPQ.empty()) {
        segmentDone = true;
        return false;
    }
    
    // Pop mejor nodo, saltar obsoletos
    PQNode node;
    do {
        if (openPQ.empty()) {
            segmentDone = true;
            return false;
        }
        node = openPQ.top();
        openPQ.pop();
    } while (closed[node.x][node.y]);
    
    int cx = node.x, cy = node.y;
    currentPos = Coord(cx, cy);
    
    // Marcar como cerrado
    closed[cx][cy] = true;
    stateGrid[cx][cy] = CLOSED;
    
    // ¿Llegamos al objetivo?
    if (cx == currentGoal.x && cy == currentGoal.y) {
        reconstructSegment();
        
        // Recolectar tesoro si hay uno
        if (challenges.hasTreasure(currentGoal)) {
            challenges.collectTreasure(currentGoal);
            stateGrid[cx][cy] = TREASURE_COLLECTED;
        }
        
        segmentDone = true;
        return false;
    }
    
    // Expandir vecinos (4 direcciones)
    for (int dir = 0; dir < 4; ++dir) {
        if (g.at(cx, cy).walls[dir]) continue;
        
        int nx = cx, ny = cy;
        if (dir == 0) ny -= 1;      // Up
        else if (dir == 1) nx -= 1;  // Left
        else if (dir == 2) nx += 1;  // Right
        else if (dir == 3) ny += 1;  // Down
        
        if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
        if (closed[nx][ny]) continue;
        
        float tentative_g = gScore[cx][cy] + 1.0f;
        
        if (tentative_g < gScore[nx][ny]) {
            cameFrom[nx][ny] = Coord(cx, cy);
            gScore[nx][ny] = tentative_g;
            fScore[nx][ny] = tentative_g + heuristic(nx, ny, currentGoal);
            openPQ.push({fScore[nx][ny], pushCounter++, nx, ny});
            stateGrid[nx][ny] = OPEN;
        }
    }
    
    return false;
}

void CollectorSolver::reconstructSegment() {
    currentSegmentPath.clear();
    Coord cur = currentGoal;
    
    // Retroceder desde meta hasta inicio usando cameFrom
    while (!(cur.x == -1 && cur.y == -1)) {
        currentSegmentPath.push_back(cur);
        cur = cameFrom[cur.x][cur.y];
    }
    
    std::reverse(currentSegmentPath.begin(), currentSegmentPath.end());
    
    // Agregar al camino completo (evitar duplicar inicio del segmento)
    for (size_t i = (fullPath.empty() ? 0 : 1); i < currentSegmentPath.size(); ++i) {
        fullPath.push_back(currentSegmentPath[i]);
    }
}

bool CollectorSolver::finished() const {
    return allDone;
}

bool CollectorSolver::getCurrent(Coord& out) const {
    if (allDone) return false;
    out = currentPos;
    return true;
}

const std::vector<Coord>& CollectorSolver::getFullPath() const {
    return fullPath;
}

int CollectorSolver::getTreasuresCollected() const {
    return challenges.collectedCount;
}