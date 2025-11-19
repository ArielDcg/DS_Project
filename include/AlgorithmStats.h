#pragma once
#include <string>

struct AlgorithmStats {
    std::string name;
    double lastTimeMs = 0.0;
    int lastNodesExplored = 0;
    int lastPathLength = 0;

    int runs = 0;
    double sumTimeMs = 0.0;
    long long sumNodes = 0;
    long long sumPathLen = 0;

    double avgTimeMs() const { return runs ? sumTimeMs / runs : 0.0; }
    double avgNodes() const { return runs ? (double)sumNodes / runs : 0.0; }
    double avgPathLen() const { return runs ? (double)sumPathLen / runs : 0.0; }

    double efficiencyScore() const {
        constexpr double wTime = 0.5;
        constexpr double wNodes = 0.4;
        constexpr double wPath = 0.1;
        return wTime * avgTimeMs() + wNodes * avgNodes() + wPath * avgPathLen();
    }
};
