#pragma once
#include "Grid.h"
#include <vector>
#include <optional>
#include <random>

/**
 * Sistema de Portales Bidireccionales (Tipo ATAJO)
 * Cada portal conecta dos puntos A y B del laberinto.
 * Si estás en A, puedes saltar a B (y viceversa).
 */
struct Portal {
    Coord a;  // Punto A
    Coord b;  // Punto B
};

class PortalSystem {
public:
    PortalSystem(int gridW, int gridH);
    
    // Generar portales aleatorios evitando start/goal
    void generatePortals(const Coord& start, const Coord& goal, int count = 2);
    
    // Si pos es entrada de un portal, retorna su salida
    std::optional<Coord> getPortalExit(const Coord& pos) const;
    
    // Para visualización
    const std::vector<Portal>& getPortals() const { return portals; }
    
    // Verificar si una posición es parte de un portal
    bool isPortalCell(const Coord& pos) const;
    
private:
    int width, height;
    std::vector<Portal> portals;
    std::mt19937 rng;
};
