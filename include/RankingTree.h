#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

// =====================
// Métricas por algoritmo
// =====================
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

    // Score combinado: menor es mejor
    double efficiencyScore() const {
        constexpr double wTime = 0.5;
        constexpr double wNodes = 0.4;
        constexpr double wPath = 0.1;
        return wTime * avgTimeMs() + wNodes * avgNodes() + wPath * avgPathLen();
    }
};

// =====================
// Modos de ranking
// =====================
enum class RankingMode { ByAvgTime, ByAvgNodes, ByEfficiency };

// =====================
// Clave de ordenamiento
// =====================
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

// =====================
// Nodo AVL genérico
// =====================
template<typename K, typename V>
struct AvlNode {
    K key;
    V value;
    int height = 1;
    AvlNode* left = nullptr;
    AvlNode* right = nullptr;
    explicit AvlNode(const K& k, const V& v) : key(k), value(v) {}
};

// =====================
// Árbol AVL genérico
// =====================
template<typename K, typename V>
class AvlTree {
public:
    using Node = AvlNode<K,V>;
    ~AvlTree() { clear(root); }

    void insert(const K& key, const V& value) { root = insertRec(root, key, value); }
    void upsert(const K& key, const V& value) { root = upsertRec(root, key, value); }

    void getTopK(int k, std::vector<V>& out) const {
        out.clear();
        out.reserve(k);
        inorderTopK(root, k, out);
    }

    template<typename F>
    void rebuild(const std::vector<V>& values, F keyOf) {
        clear(root); root = nullptr;
        for (const auto& v : values) insert(keyOf(v), v);
    }

    bool empty() const { return root == nullptr; }

private:
    Node* root = nullptr;

    static int h(Node* n) { return n ? n->height : 0; }
    static int bal(Node* n) { return n ? h(n->left) - h(n->right) : 0; }
    static void update(Node* n) { n->height = 1 + std::max(h(n->left), h(n->right)); }

    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;
        x->right = y; y->left = T2;
        update(y); update(x);
        return x;
    }
    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;
        y->left = x; x->right = T2;
        update(x); update(y);
        return y;
    }
    static Node* balance(Node* n) {
        update(n);
        int b = bal(n);
        if (b > 1) {
            if (bal(n->left) < 0) n->left = rotateLeft(n->left);
            return rotateRight(n);
        }
        if (b < -1) {
            if (bal(n->right) > 0) n->right = rotateRight(n->right);
            return rotateLeft(n);
        }
        return n;
    }
    static Node* insertRec(Node* n, const K& key, const V& value) {
        if (!n) return new Node(key, value);
        if (key < n->key) n->left = insertRec(n->left, key, value);
        else n->right = insertRec(n->right, key, value);
        return balance(n);
    }
    static Node* upsertRec(Node* n, const K& key, const V& value) {
        if (!n) return new Node(key, value);
        if (key == n->key) { n->value = value; return n; }
        if (key < n->key) n->left = upsertRec(n->left, key, value);
        else n->right = upsertRec(n->right, key, value);
        return balance(n);
    }
    static void inorderTopK(Node* n, int k, std::vector<V>& out) {
        if (!n || (int)out.size() >= k) return;
        inorderTopK(n->left, k, out);
        if ((int)out.size() < k) out.push_back(n->value);
        inorderTopK(n->right, k, out);
    }
    static void clear(Node* n) {
        if (!n) return;
        clear(n->left); clear(n->right);
        delete n;
    }
};

// =====================
// Clase RankingTree
// =====================
class RankingTree {
public:
    explicit RankingTree(RankingMode mode = RankingMode::ByEfficiency) : mode_(mode) {}

    void setMode(RankingMode m) { mode_ = m; rebuildIndex(); }

    void recordRun(const std::string& algName, double timeMs, int nodesExplored, int pathLength) {
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

    std::vector<AlgorithmStats> topK(int k) const {
        std::vector<AlgorithmStats> out;
        avl_.getTopK(k, out);
        return out;
    }

    std::vector<AlgorithmStats> all() const {
        std::vector<AlgorithmStats> out;
        avl_.getTopK((int)stats_.size(), out);
        return out;
    }

    void exportCSV(const std::string& path) const {
        std::ofstream f(path);
        f << "Algorithm,Runs,AvgTimeMs,AvgNodes,AvgPathLen,Efficiency\n";
        for (auto& s : all()) {
            f << s.name << "," << s.runs << ","
              << s.avgTimeMs() << "," << s.avgNodes() << ","
              << s.avgPathLen() << "," << s.efficiencyScore() << "\n";
        }
    }

private:
    RankingMode mode_;
    std::unordered_map<std::string, AlgorithmStats> stats_;
    AvlTree<RankingKey, AlgorithmStats> avl_;

    RankingKey keyOf(const AlgorithmStats& s) const {
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

    void rebuildIndex() {
        std::vector<AlgorithmStats> vals;
        vals.reserve(stats_.size());
        for (auto& [_, s] : stats_) vals.push_back(s);
        avl_.rebuild(vals, [this](const AlgorithmStats& s){ return keyOf(s); });
    }
};
