#include "PortalSystem.h"
#include <algorithm>

PortalSystem::PortalSystem(int gridW, int gridH)
    : width(gridW), height(gridH), rng(std::random_device{}())
{
}

void PortalSystem::generatePortals(const Coord& start, const Coord& goal, int count) {
    portals.clear();
    
    std::uniform_int_distribution<int> distX(0, width - 1);
    std::uniform_int_distribution<int> distY(0, height - 1);
    
    for (int i = 0; i < count; ++i) {
        Portal p;
        int attempts = 0;
        const int maxAttempts = 100;
        
        do {
            p.a = Coord(distX(rng), distY(rng));
            p.b = Coord(distX(rng), distY(rng));
            attempts++;
            
            // Verificar que no sean iguales, no sean start/goal,
            // y no colisionen con otros portales
            bool valid = !(p.a == p.b) &&
                         !(p.a == start) && !(p.a == goal) &&
                         !(p.b == start) && !(p.b == goal);
            
            // Verificar que no colisione con portales existentes
            if (valid) {
                for (const auto& existing : portals) {
                    if (p.a == existing.a || p.a == existing.b ||
                        p.b == existing.a || p.b == existing.b) {
                        valid = false;
                        break;
                    }
                }
            }
            
            // Asegurar distancia mínima entre puntos del portal (para que sea útil)
            if (valid) {
                int dist = std::abs(p.a.x - p.b.x) + std::abs(p.a.y - p.b.y);
                if (dist < 5) valid = false; // Distancia mínima Manhattan de 5
            }
            
            if (valid) break;
            
        } while (attempts < maxAttempts);
        
        if (attempts < maxAttempts) {
            portals.push_back(p);
        }
    }
}

std::optional<Coord> PortalSystem::getPortalExit(const Coord& pos) const {
    for (const auto& p : portals) {
        if (pos == p.a) return p.b;
        if (pos == p.b) return p.a;
    }
    return std::nullopt;
}

bool PortalSystem::isPortalCell(const Coord& pos) const {
    for (const auto& p : portals) {
        if (pos == p.a || pos == p.b) return true;
    }
    return false;
}
