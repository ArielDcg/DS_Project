#pragma once
#include <string>

struct SolverMetrics {
    std::string name;
    int nodesExplored = 0;
    double timeMs = 0.0;
    int pathLength = 0;

    // puntuación compuesta: menor es mejor
    double efficiencyScore(double w_time = 1.0, double w_nodes = 0.001, double w_path = 0.1) const {
        return w_time * timeMs + w_nodes * nodesExplored + w_path * pathLength;
    }
};
