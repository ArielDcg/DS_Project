#pragma once
#include "Grid.h"
#include <vector>
#include <unordered_map>
#include <queue>

/**
 * Análisis de Grafos para el laberinto.
 * Construye lista de adyacencia y calcula el diámetro (camino más largo).
 */
struct GraphStats {
    int diameter;              // Longitud del camino más largo
    Coord diameterStart;       // Punto inicial del diámetro
    Coord diameterEnd;         // Punto final del diámetro
    std::vector<Coord> path;   // Camino del diámetro
};

class GraphAnalysis {
public:
    GraphAnalysis(const Grid& grid);
    
    // Construir lista de adyacencia desde el laberinto
    void buildAdjacencyList();
    
    // Calcular diámetro usando doble BFS
    GraphStats calculateDiameter();
    
private:
    const Grid& g;
    
    // Lista de adyacencia: nodeId -> [vecinos conectados]
    std::unordered_map<int, std::vector<int>> adjList;
    
    // Conversión Coord <-> ID
    int coordToId(int x, int y) const;
    Coord idToCoord(int id) const;
    
    // BFS que retorna el nodo más lejano y su distancia
    std::pair<int, int> bfsFarthest(int startId);
    
    // Reconstruir camino entre dos nodos
    std::vector<Coord> reconstructPath(int fromId, int toId);
};
