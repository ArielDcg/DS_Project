#pragma once
#include "Grid.h"
#include <vector>
#include <random>

/**
 * Origin Shift con Múltiples Raíces: El laberinto es representado como un GRAFO
 * usando lista de adyacencia implícita (parentMap). Múltiples raíces se mueven
 * simultáneamente, haciendo que el laberinto cambie más rápido en diferentes zonas.
 * 
 * ESTRUCTURA DE DATOS: Lista de Adyacencia (representada como parentMap)
 * Cada celda apunta a su padre en el árbol dirigido. Las raíces no tienen padre.
 */
class OriginShiftMaze {
public:
    OriginShiftMaze(Grid& grid, int numRoots = 3);
    
    // Mover TODAS las raíces a vecinos aleatorios e invertir aristas
    void update();
    
    // Obtener posiciones de todas las raíces
    const std::vector<Coord>& getRoots() const { return roots; }
    
    // Sincronizar paredes del Grid según parentMap
    void applyToGrid();
    
    // Inicializar el árbol dirigido desde el laberinto existente
    void initializeFromMaze();
    
    // Estadísticas del grafo
    int getNodeCount() const { return g.width() * g.height(); }
    int getEdgeCount() const;  // Número de aristas (conexiones sin pared)
    
private:
    Grid& g;
    std::vector<Coord> roots;  // Múltiples raíces
    int numRoots;
    
    // parentMap[x][y] = dirección hacia el padre (0=Up, 1=Left, 2=Right, 3=Down)
    // -1 significa que es una raíz
    std::vector<std::vector<int>> parentMap;
    
    std::mt19937 rng;
    
    // Obtener vecinos válidos (celdas adyacentes dentro del grid)
    std::vector<std::pair<Coord, int>> getNeighbors(const Coord& c) const;
    
    // Dirección opuesta: Up<->Down, Left<->Right
    int oppositeDir(int dir) const;
    
    // Mover una raíz específica
    void updateSingleRoot(int rootIndex);
};
