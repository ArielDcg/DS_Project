#include "RankingManager.h"
#include <sstream>
#include <algorithm>

RankingManager::RankingManager() {
    criterion = Criterion::TIME;
    updateComparator();
}

void RankingManager::updateComparator() {
    switch (criterion) {
        case Criterion::TIME:
            comparator = [](const SolverMetrics &a, const SolverMetrics &b){ return a.timeMs < b.timeMs; };
            break;
        case Criterion::NODES:
            comparator = [](const SolverMetrics &a, const SolverMetrics &b){ return a.nodesExplored < b.nodesExplored; };
            break;
        case Criterion::PATH:
            comparator = [](const SolverMetrics &a, const SolverMetrics &b){ return a.pathLength < b.pathLength; };
            break;
        case Criterion::EFFICIENCY:
            comparator = [](const SolverMetrics &a, const SolverMetrics &b){
                return a.efficiencyScore() < b.efficiencyScore();
            };
            break;
    }
}

void RankingManager::setCriterion(Criterion c) {
    if (criterion == c) return;
    criterion = c;
    updateComparator();
}

void RankingManager::add(const SolverMetrics &m) {
    store.push_back(m);
}

std::vector<SolverMetrics> RankingManager::getOrdered() const {
    std::vector<SolverMetrics> out = store;
    std::sort(out.begin(), out.end(), comparator);
    return out;
}

std::string RankingManager::toString() const {
    std::ostringstream oss;
    auto ordered = getOrdered();
    oss << "Ranking (criterion = ";
    switch (criterion) {
        case Criterion::TIME: oss << "TIME"; break;
        case Criterion::NODES: oss << "NODES"; break;
        case Criterion::PATH: oss << "PATH"; break;
        case Criterion::EFFICIENCY: oss << "EFFICIENCY"; break;
    }
    oss << ")\n";
    oss << "Rank | Name     | time(ms) | nodes | pathLen | score\n";
    int r = 1;
    for (const auto &m : ordered) {
        oss << r++ << " | " << m.name << " | " << m.timeMs << " | " << m.nodesExplored << " | " << m.pathLength
            << " | " << m.efficiencyScore() << "\n";
    }
    return oss.str();
}
