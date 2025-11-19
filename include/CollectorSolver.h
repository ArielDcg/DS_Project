#pragma once
#include "Grid.h"
#include "ChallengeSystem.h"
#include <vector>
#include <queue>
#include <limits>

// ===================================
// COLLECTOR SOLVER
// ===================================
// Resuelve laberinto recolectando tesoros secuencialmente
// Usa greedy mejorado para ordenar objetivos
// Usa A* para navegar entre cada par de puntos

class CollectorSolver {
public:
    CollectorSolver(Grid& grid, ChallengeSystem& challenges, Coord start, Coord finalGoal);
    
    // Ejecutar un paso del algoritmo
    bool step();
    
    // ¿Ya terminó?
    bool finished() const;
    
    // Obtener posición actual (para visualización)
    bool getCurrent(Coord& out) const;
    
    // Obtener camino completo acumulado
    const std::vector<Coord>& getFullPath() const;
    
    // Número de tesoros recolectados
    int getTreasuresCollected() const;
    
    // Índice del objetivo actual (0-3)
    int getCurrentObjectiveIndex() const { return currentObjectiveIndex; }

    // Estados para visualización
    enum CellState { UNKNOWN = 0, OPEN = 1, CLOSED = 2, TREASURE_COLLECTED = 3 };
    const std::vector<std::vector<CellState>>& getStateGrid() const { return stateGrid; }
    const std::vector<std::vector<float>>& getGScoreGrid() const { return gScore; }

private:
    Grid& g;
    ChallengeSystem& challenges;
    Coord start;
    Coord finalGoal;
    
    // Lista ordenada de objetivos
    std::vector<Coord> objectives;
    int currentObjectiveIndex = 0;
    Coord currentGoal;
    
    // Estructuras A* interno
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
    
    Coord currentPos;
    std::vector<Coord> currentSegmentPath;
    std::vector<Coord> fullPath;
    
    bool segmentDone = false;
    bool allDone = false;
    
    // Funciones auxiliares
    void initializeSearch(Coord from, Coord to);
    bool stepAStar();
    void reconstructSegment();
    float heuristic(int x, int y, const Coord& goal) const;
    float manhattan(const Coord& a, const Coord& b) const;
    
    // Greedy mejorado: ordena tesoros considerando distancia actual + distancia a meta
    std::vector<Coord> greedyOrderTreasures(Coord from, const std::vector<Coord>& treasures, Coord goal);
};