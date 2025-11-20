#pragma once
#include "Grid.h"
#include "ChallengeSystem.h"
#include <vector>
#include <queue>
#include <limits>

// ===================================
// ESTRATEGIAS DE PATHFINDING
// ===================================
enum class SolverStrategy {
    ASTAR,      // f = g + h (costo + heurística Manhattan)
    GREEDY,     // f = h (solo heurística)
    UCS,        // f = g (solo costo, Dijkstra)
    DFS         // Depth-first search (stack)
};

// ===================================
// SEGMENTO DE CAMINO
// Para mantener colores persistentes
// ===================================
struct PathSegment {
    std::vector<Coord> path;
    int objectiveIndex;  // 0=T1, 1=T2, 2=T3, 3=Meta
    bool foundBonus = false;  // ¿Encontró tesoro de paso?
    Coord bonusTreasure;      // Posición del tesoro bonus
    
    PathSegment() : objectiveIndex(0), foundBonus(false) {}
};

// ===================================
// COLLECTOR SOLVER
// ===================================
class CollectorSolver {
public:
    CollectorSolver(Grid& grid, ChallengeSystem& challenges, 
                    Coord start, Coord finalGoal,
                    SolverStrategy strategy = SolverStrategy::ASTAR);
    
    // Ejecutar un paso del algoritmo
    bool step();
    
    // ¿Ya terminó?
    bool finished() const;
    
    // Obtener posición actual
    bool getCurrent(Coord& out) const;
    
    // Obtener camino completo acumulado (legacy)
    const std::vector<Coord>& getFullPath() const;
    
    // ===================================
    // 🆕 NUEVO: Obtener segmentos con colores
    // ===================================
    const std::vector<PathSegment>& getSegments() const;
    
    // Número de tesoros recolectados
    int getTreasuresCollected() const;
    
    // Índice del objetivo actual
    int getCurrentObjectiveIndex() const { return currentObjectiveIndex; }
    
    // Estrategia actual
    SolverStrategy getStrategy() const { return strategy; }

    // Estados para visualización
    enum CellState { UNKNOWN = 0, OPEN = 1, CLOSED = 2, TREASURE_COLLECTED = 3 };
    const std::vector<std::vector<CellState>>& getStateGrid() const { return stateGrid; }
    const std::vector<std::vector<float>>& getGScoreGrid() const { return gScore; }

private:
    Grid& g;
    ChallengeSystem& challenges;
    Coord start;
    Coord finalGoal;
    SolverStrategy strategy;
    
    // Lista ordenada de objetivos
    std::vector<Coord> objectives;
    int currentObjectiveIndex = 0;
    Coord currentGoal;
    
    // Estructuras A*/Greedy/UCS compartidas
    std::vector<std::vector<CellState>> stateGrid;
    std::vector<std::vector<bool>> closed;
    std::vector<std::vector<float>> gScore;
    std::vector<std::vector<float>> fScore;
    std::vector<std::vector<Coord>> cameFrom;
    
    struct PQNode { float f; int id; int x; int y; };
    struct Compare {
        bool operator()(const PQNode& a, const PQNode& b) const {
            if (a.f == b.f) return a.id > b.id;
            return a.f > b.f;
        }
    };
    std::priority_queue<PQNode, std::vector<PQNode>, Compare> openPQ;
    int pushCounter = 0;
    
    // DFS específico
    std::vector<Coord> dfsStack;
    std::vector<std::vector<bool>> dfsVisited;
    
    Coord currentPos;
    std::vector<Coord> currentSegmentPath;
    std::vector<Coord> fullPath;  // Legacy
    
    // ===================================
    // 🆕 NUEVO: Segmentos separados
    // ===================================
    std::vector<PathSegment> segments;
    PathSegment currentSegment;
    
    bool segmentDone = false;
    bool allDone = false;
    
    // Funciones auxiliares
    void initializeSearch(Coord from, Coord to);
    void initializeDFS(Coord from, Coord to);
    
    bool stepAStar();
    bool stepGreedy();
    bool stepUCS();
    bool stepDFS();
    
    void reconstructSegment();
    
    float heuristic(int x, int y, const Coord& goal) const;
    float manhattan(const Coord& a, const Coord& b) const;
    
    // ===================================
    // 🆕 NUEVO: Recolección oportunista
    // ===================================
    void checkOpportunisticCollection();
    void removeFromObjectives(const Coord& pos);
    
    // Greedy mejorado
    std::vector<Coord> greedyOrderTreasures(Coord from, const std::vector<Coord>& treasures, Coord goal);
};