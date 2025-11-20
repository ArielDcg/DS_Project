#pragma once
#include "Metrics.h"
#include "AVLTree.h"
#include <vector>
#include <functional>
#include <string>

class RankingManager {
public:
    enum class Criterion { TIME, NODES, PATH, EFFICIENCY };

    RankingManager();

    void setCriterion(Criterion c);
    Criterion getCriterion() const { return criterion; }

    void add(const SolverMetrics &m);
    // devuelve ordenado de mejor (mejor = menor) a peor
    std::vector<SolverMetrics> getOrdered() const;

    // export helpers
    std::string toString() const;

private:
    Criterion criterion;
    // The tree is recreated when criterion changes or when needed
    std::vector<SolverMetrics> store; // keep all metrics; tree is build-on-demand
    std::function<bool(const SolverMetrics&, const SolverMetrics&)> comparator;
    void updateComparator();
};
