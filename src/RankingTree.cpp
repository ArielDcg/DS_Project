#include "RankingTree.h"

RankingTree::RankingTree(RankingMode mode) : mode_(mode) {}

void RankingTree::setMode(RankingMode m) {
    mode_ = m;
    rebuildIndex();
}

void RankingTree::recordRun(const std::string& algName, double timeMs, int nodesExplored, int pathLength) {
    auto& s = stats_[algName];
    s.name = algName;
    s.lastTimeMs = timeMs;
    s.lastNodesExplored = nodesExplored;
    s.lastPathLength = pathLength;
    s.runs++;
    s.sumTimeMs += timeMs;
    s.sumNodes += nodesExplored;
    s.sumPathLen += pathLength;
    avl_.upsert(keyOf(s), s);
}

std::vector<AlgorithmStats> RankingTree::topK(int k) const {
    std::vector<AlgorithmStats> out;
    avl_.getTopK(k, out);
    return out;
}

std::vector<AlgorithmStats> RankingTree::all() const {
    std::vector<AlgorithmStats> out;
    avl_.getTopK((int)stats_.size(), out);
    return out;
}

void RankingTree::exportCSV(const std::string& path) const {
    std::ofstream f(path);
    f << "Algorithm,Runs,AvgTimeMs,AvgNodes,AvgPathLen,Efficiency\n";
    for (auto& s : all()) {
        f << s.name << "," << s.runs << ","
          << s.avgTimeMs() << "," << s.avgNodes() << ","
          << s.avgPathLen() << "," << s.efficiencyScore() << "\n";
    }
}

RankingKey RankingTree::keyOf(const AlgorithmStats& s) const {
    switch (mode_) {
        case RankingMode::ByAvgTime:
            return { s.avgTimeMs(), s.efficiencyScore(), s.avgNodes(), s.name };
        case RankingMode::ByAvgNodes:
            return { s.avgNodes(), s.efficiencyScore(), s.avgTimeMs(), s.name };
        case RankingMode::ByEfficiency:
        default:
            return { s.efficiencyScore(), s.avgTimeMs(), s.avgNodes(), s.name };
    }
}

void RankingTree::rebuildIndex() {
    std::vector<AlgorithmStats> vals;
    vals.reserve(stats_.size());
    for (auto& [_, s] : stats_) vals.push_back(s);
    avl_.rebuild(vals, [this](const AlgorithmStats& s){ return keyOf(s); });
}

