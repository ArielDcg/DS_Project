#pragma once
#include "AlgorithmStats.h"
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

enum class RankingMode { ByAvgTime, ByAvgNodes, ByEfficiency };

struct RankingKey {
    double primary;
    double tie1;
    double tie2;
    std::string name;

    bool operator<(const RankingKey& o) const {
        if (primary != o.primary) return primary < o.primary;
        if (tie1 != o.tie1) return tie1 < o.tie1;
        if (tie2 != o.tie2) return tie2 < o.tie2;
        return name < o.name;
    }
    bool operator==(const RankingKey& o) const {
        return primary == o.primary && tie1 == o.tie1 && tie2 == o.tie2 && name == o.name;
    }
};

// Nodo AVL
template<typename K, typename V>
struct AvlNode {
    K key;
    V value;
    int height = 1;
    AvlNode* left = nullptr;
    AvlNode* right = nullptr;
    explicit AvlNode(const K& k, const V& v) : key(k), value(v) {}
};

// Árbol AVL
template<typename K, typename V>
class AvlTree {
    using Node = AvlNode<K,V>;
    Node* root = nullptr;
    // ... (rotaciones, balanceo, inserción como te mostré antes)
};
