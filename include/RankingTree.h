#pragma once

#include "AlgorithmStats.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

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

template<typename K, typename V>
struct AvlNode {
    K key;
    V value;
    int height = 1;
    AvlNode* left = nullptr;
    AvlNode* right = nullptr;
    explicit AvlNode(const K& k, const V& v) : key(k), value(v) {}
};

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

class RankingTree {
public:
    explicit RankingTree(RankingMode mode = RankingMode::ByEfficiency);

    void setMode(RankingMode m);
    void recordRun(const std::string& algName, double timeMs, int nodesExplored, int pathLength);

    std::vector<AlgorithmStats> topK(int k) const;
    std::vector<AlgorithmStats> all() const;

    void exportCSV(const std::string& path) const;

private:
    RankingMode mode_;
    std::unordered_map<std::string, AlgorithmStats> stats_;
    AvlTree<RankingKey, AlgorithmStats> avl_;

    RankingKey keyOf(const AlgorithmStats& s) const;
    void rebuildIndex();
};

