#include "CollectorSolver.h"
#include <cmath>
#include <algorithm>
#include <random>

CollectorSolver::CollectorSolver(Grid& grid, ChallengeSystem& chall, Coord s, Coord fg, SolverStrategy strat)
    : g(grid), challenges(chall), start(s), finalGoal(fg), strategy(strat),
      heatmap(grid.width(), grid.height()),  // ✅ Inicializar heatmap (ESTRUCTURA 2/3)
      stateGrid(grid.width(), std::vector<CellState>(grid.height(), UNKNOWN)),
      closed(grid.width(), std::vector<bool>(grid.height(), false)),
      gScore(grid.width(), std::vector<float>(grid.height(), std::numeric_limits<float>::infinity())),
      fScore(grid.width(), std::vector<float>(grid.height(), std::numeric_limits<float>::infinity())),
      cameFrom(grid.width(), std::vector<Coord>(grid.height(), Coord(-1,-1))),
      dfsVisited(grid.width(), std::vector<bool>(grid.height(), false)),
      currentPos(s) {
    
    // Greedy mejorado: ordenar tesoros
    objectives = greedyOrderTreasures(start, challenges.getTreasurePositions(), finalGoal);
    objectives.push_back(finalGoal);
    
    currentSegment.objectiveIndex = 0;
    
    if (!objectives.empty()) {
        currentGoal = objectives[0];
        if (strategy == SolverStrategy::DFS) {
            initializeDFS(start, currentGoal);
        } else {
            initializeSearch(start, currentGoal);
        }
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
        // Buscar MÁXIMO (tesoro más alejado de meta)
        float bestScore = -std::numeric_limits<float>::infinity();
        int bestIdx = 0;
        
        for (size_t i = 0; i < remaining.size(); ++i) {
            float toTreasure = manhattan(current, remaining[i]);
            float treasureToGoal = manhattan(remaining[i], goal);
            float totalScore = toTreasure + treasureToGoal;
            
            // ✅ Buscar MÁXIMO
            if (totalScore > bestScore) {
                bestScore = totalScore;
                bestIdx = static_cast<int>(i);
            }
        }
        
        ordered.push_back(remaining[bestIdx]);
        current = remaining[bestIdx];
        remaining.erase(remaining.begin() + bestIdx);
    }
    
    return ordered;
}

void CollectorSolver::initializeSearch(Coord from, Coord to) {
    // Limpieza completa
    closed.assign(g.width(), std::vector<bool>(g.height(), false));
    gScore.assign(g.width(), std::vector<float>(g.height(), std::numeric_limits<float>::infinity()));
    fScore.assign(g.width(), std::vector<float>(g.height(), std::numeric_limits<float>::infinity()));
    cameFrom.assign(g.width(), std::vector<Coord>(g.height(), Coord(-1,-1)));
    stateGrid.assign(g.width(), std::vector<CellState>(g.height(), UNKNOWN));
    
    while (!openPQ.empty()) openPQ.pop();
    pushCounter = 0;
    
    currentPos = from;
    currentGoal = to;
    gScore[from.x][from.y] = 0.0f;
    fScore[from.x][from.y] = heuristic(from.x, from.y, to);
    openPQ.push({fScore[from.x][from.y], pushCounter++, from.x, from.y});
    stateGrid[from.x][from.y] = OPEN;
    
    segmentDone = false;
    currentSegmentPath.clear();
}

void CollectorSolver::initializeDFS(Coord from, Coord to) {
    dfsVisited.assign(g.width(), std::vector<bool>(g.height(), false));
    stateGrid.assign(g.width(), std::vector<CellState>(g.height(), UNKNOWN));
    
    dfsStack.clear();
    dfsStack.push_back(from);
    dfsVisited[from.x][from.y] = true;
    stateGrid[from.x][from.y] = OPEN;
    
    currentPos = from;
    currentGoal = to;
    segmentDone = false;
    currentSegmentPath.clear();
}

float CollectorSolver::heuristic(int x, int y, const Coord& goal) const {
    return static_cast<float>(std::abs(goal.x - x) + std::abs(goal.y - y));
}

void CollectorSolver::checkOpportunisticCollection() {
    if (challenges.hasTreasure(currentPos)) {
        // ¿Es un tesoro que NO es mi objetivo actual?
        if (!(currentPos.x == currentGoal.x && currentPos.y == currentGoal.y)) {
            // ¡Tesoro de paso encontrado!
            challenges.collectTreasure(currentPos);
            stateGrid[currentPos.x][currentPos.y] = TREASURE_COLLECTED;
            
            // Marcar en el segmento actual
            currentSegment.foundBonus = true;
            currentSegment.bonusTreasure = currentPos;
            
            // Remover de objetivos futuros
            removeFromObjectives(currentPos);
        }
    }
}

void CollectorSolver::removeFromObjectives(const Coord& pos) {
    for (size_t i = currentObjectiveIndex + 1; i < objectives.size(); ++i) {
        if (objectives[i].x == pos.x && objectives[i].y == pos.y) {
            objectives.erase(objectives.begin() + i);
            break;
        }
    }
}
// CollectorSolver.cpp PARTE 2 - Continúa de part1

bool CollectorSolver::step() {
    if (allDone) return true;
    
    if (segmentDone) {
        // Guardar segmento completado
        segments.push_back(currentSegment);
        
        // Pasar al siguiente objetivo
        currentObjectiveIndex++;
        
        if (currentObjectiveIndex >= static_cast<int>(objectives.size())) {
            allDone = true;
            return true;
        }
        
        // Preparar nuevo segmento
        currentSegment = PathSegment();
        currentSegment.objectiveIndex = currentObjectiveIndex;
        
        Coord nextGoal = objectives[currentObjectiveIndex];
        if (strategy == SolverStrategy::DFS) {
            initializeDFS(currentPos, nextGoal);
        } else {
            initializeSearch(currentPos, nextGoal);
        }
        return false;
    }

    switch (strategy) {
        case SolverStrategy::ASTAR:
            return stepAStar();
        case SolverStrategy::GREEDY:
            return stepGreedy();
        case SolverStrategy::UCS:
            return stepUCS();
        case SolverStrategy::DFS:
            return stepDFS();
    }
    
    return false;
}

bool CollectorSolver::stepAStar() {
    if (openPQ.empty()) {
        segmentDone = true;
        return false;
    }
    
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
    
    closed[cx][cy] = true;
    stateGrid[cx][cy] = CLOSED;

    heatmap.recordVisit(cx, cy);

    checkOpportunisticCollection();
    
    // ¿Llegamos al objetivo?
    if (cx == currentGoal.x && cy == currentGoal.y) {
        reconstructSegment();
        
        if (challenges.hasTreasure(currentGoal)) {
            challenges.collectTreasure(currentGoal);
            stateGrid[cx][cy] = TREASURE_COLLECTED;
        }
        
        segmentDone = true;
        return false;
    }
    
    // Expandir vecinos
    for (int dir = 0; dir < 4; ++dir) {
        if (g.at(cx, cy).walls[dir]) continue;
        
        int nx = cx, ny = cy;
        if (dir == 0) ny -= 1;
        else if (dir == 1) nx -= 1;
        else if (dir == 2) nx += 1;
        else if (dir == 3) ny += 1;
        
        if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
        if (closed[nx][ny]) continue;
        
        float tentative_g = gScore[cx][cy] + 1.0f;
        
        if (tentative_g < gScore[nx][ny]) {
            cameFrom[nx][ny] = Coord(cx, cy);
            gScore[nx][ny] = tentative_g;
            fScore[nx][ny] = tentative_g + heuristic(nx, ny, currentGoal);
            
            // ✅ Solo agregar si es nuevo (no está en OPEN ni CLOSED)
            if (stateGrid[nx][ny] == UNKNOWN) {
                openPQ.push({fScore[nx][ny], pushCounter++, nx, ny});
                stateGrid[nx][ny] = OPEN;
            }
            // Si ya está OPEN: scores actualizados, pero nodo ya en cola
            // La próxima vez que se procese usará los nuevos scores
        }
    }
    
    return false;
}

bool CollectorSolver::stepGreedy() {
    if (openPQ.empty()) {
        segmentDone = true;
        return false;
    }
    
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
    
    closed[cx][cy] = true;
    stateGrid[cx][cy] = CLOSED;
    
    heatmap.recordVisit(cx, cy); 
    
    checkOpportunisticCollection();
    
    if (cx == currentGoal.x && cy == currentGoal.y) {
        reconstructSegment();
        
        if (challenges.hasTreasure(currentGoal)) {
            challenges.collectTreasure(currentGoal);
            stateGrid[cx][cy] = TREASURE_COLLECTED;
        }
        
        segmentDone = true;
        return false;
    }
    
    for (int dir = 0; dir < 4; ++dir) {
        if (g.at(cx, cy).walls[dir]) continue;
        
        int nx = cx, ny = cy;
        if (dir == 0) ny -= 1;
        else if (dir == 1) nx -= 1;
        else if (dir == 2) nx += 1;
        else if (dir == 3) ny += 1;
        
        if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
        if (closed[nx][ny]) continue;
        
        float tentative_g = gScore[cx][cy] + 1.0f;
        
        if (tentative_g < gScore[nx][ny]) {
            cameFrom[nx][ny] = Coord(cx, cy);
            gScore[nx][ny] = tentative_g;
            fScore[nx][ny] = heuristic(nx, ny, currentGoal);
            
            // ✅ Solo agregar si es nuevo
            if (stateGrid[nx][ny] == UNKNOWN) {
                openPQ.push({fScore[nx][ny], pushCounter++, nx, ny});
                stateGrid[nx][ny] = OPEN;
            }
        }
    }
    
    return false;
}

bool CollectorSolver::stepUCS() {
    if (openPQ.empty()) {
        segmentDone = true;
        return false;
    }
    
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
    
    closed[cx][cy] = true;
    stateGrid[cx][cy] = CLOSED;
    
    heatmap.recordVisit(cx, cy);  
    
    checkOpportunisticCollection();
    
    if (cx == currentGoal.x && cy == currentGoal.y) {
        reconstructSegment();
        
        if (challenges.hasTreasure(currentGoal)) {
            challenges.collectTreasure(currentGoal);
            stateGrid[cx][cy] = TREASURE_COLLECTED;
        }
        
        segmentDone = true;
        return false;
    }
    
    for (int dir = 0; dir < 4; ++dir) {
        if (g.at(cx, cy).walls[dir]) continue;
        
        int nx = cx, ny = cy;
        if (dir == 0) ny -= 1;
        else if (dir == 1) nx -= 1;
        else if (dir == 2) nx += 1;
        else if (dir == 3) ny += 1;
        
        if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
        if (closed[nx][ny]) continue;
        
        float tentative_g = gScore[cx][cy] + 1.0f;
        
        if (tentative_g < gScore[nx][ny]) {
            cameFrom[nx][ny] = Coord(cx, cy);
            gScore[nx][ny] = tentative_g;
            fScore[nx][ny] = tentative_g;
            
            // ✅ Solo agregar si es nuevo
            if (stateGrid[nx][ny] == UNKNOWN) {
                openPQ.push({fScore[nx][ny], pushCounter++, nx, ny});
                stateGrid[nx][ny] = OPEN;
            }
        }
    }
    
    return false;
}
// CollectorSolver.cpp PARTE 3 - Continúa de part2

// ===================================
// DEPTH-FIRST SEARCH
// ===================================
bool CollectorSolver::stepDFS() {
    if (dfsStack.empty()) {
        segmentDone = true;
        return false;
    }
    
    Coord cur = dfsStack.back();
    dfsStack.pop_back();
    
    int cx = cur.x, cy = cur.y;
    currentPos = cur;
    stateGrid[cx][cy] = CLOSED;
    
    heatmap.recordVisit(cx, cy);  // ✅ Registrar en heatmap
    
    checkOpportunisticCollection();
    
    // ¿Llegamos al objetivo?
    if (cx == currentGoal.x && cy == currentGoal.y) {
        // DFS no usa cameFrom, construir camino desde stack
        currentSegmentPath.clear();
        currentSegmentPath.push_back(currentGoal);
        
        // Agregar al fullPath
        fullPath.push_back(currentGoal);
        
        // Guardar en segmento
        currentSegment.path = currentSegmentPath;
        
        if (challenges.hasTreasure(currentGoal)) {
            challenges.collectTreasure(currentGoal);
            stateGrid[cx][cy] = TREASURE_COLLECTED;
        }
        
        segmentDone = true;
        return false;
    }
    
    // Expandir vecinos en orden aleatorio (característica DFS)
    std::vector<int> dirs = {0, 1, 2, 3};
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::shuffle(dirs.begin(), dirs.end(), rng);
    
    for (int dir : dirs) {
        if (g.at(cx, cy).walls[dir]) continue;
        
        int nx = cx, ny = cy;
        if (dir == 0) ny -= 1;
        else if (dir == 1) nx -= 1;
        else if (dir == 2) nx += 1;
        else if (dir == 3) ny += 1;
        
        if (nx < 0 || ny < 0 || nx >= g.width() || ny >= g.height()) continue;
        if (dfsVisited[nx][ny]) continue;
        
        dfsVisited[nx][ny] = true;
        dfsStack.push_back(Coord(nx, ny));
        stateGrid[nx][ny] = OPEN;
    }
    
    return false;
}

void CollectorSolver::reconstructSegment() {
    currentSegmentPath.clear();
    Coord cur = currentGoal;
    
    while (!(cur.x == -1 && cur.y == -1)) {
        currentSegmentPath.push_back(cur);
        cur = cameFrom[cur.x][cur.y];
    }
    
    std::reverse(currentSegmentPath.begin(), currentSegmentPath.end());
    
    // Guardar en segmento actual
    currentSegment.path = currentSegmentPath;
    
    // Agregar al fullPath (legacy, mantener compatibilidad)
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

const std::vector<PathSegment>& CollectorSolver::getSegments() const {
    return segments;
}

int CollectorSolver::getTreasuresCollected() const {
    return challenges.collectedCount;
}