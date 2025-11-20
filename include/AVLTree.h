#pragma once
#include <vector>
#include <functional>
#include <memory>

// Simple AVL tree specialized for SolverMetrics (but templated)
template <typename T>
struct AVLNode {
    T value;
    std::unique_ptr<AVLNode<T>> left;
    std::unique_ptr<AVLNode<T>> right;
    int height = 1;
    AVLNode(const T &v) : value(v) {}
};

template <typename T, typename Compare = std::function<bool(const T&, const T&)>>
class AVLTree {
public:
    AVLTree(Compare cmp) : root(nullptr), cmp(cmp) {}
    void insert(const T &v) { root = insertImpl(std::move(root), v); }
    std::vector<T> inorder() const {
        std::vector<T> out;
        inorderImpl(root.get(), out);
        return out;
    }
private:
    std::unique_ptr<AVLNode<T>> root;
    Compare cmp;

    int height(const std::unique_ptr<AVLNode<T>> &n) const { return n ? n->height : 0; }
    int balanceFactor(const std::unique_ptr<AVLNode<T>> &n) const { return n ? height(n->left) - height(n->right) : 0; }
    void updateHeight(std::unique_ptr<AVLNode<T>> &n) { n->height = 1 + std::max(height(n->left), height(n->right)); }

    std::unique_ptr<AVLNode<T>> rotateRight(std::unique_ptr<AVLNode<T>> y) {
        auto x = std::move(y->left);
        y->left = std::move(x->right);
        x->right = std::move(y);
        updateHeight(x->right);
        updateHeight(x);
        return x;
    }

    std::unique_ptr<AVLNode<T>> rotateLeft(std::unique_ptr<AVLNode<T>> x) {
        auto y = std::move(x->right);
        x->right = std::move(y->left);
        y->left = std::move(x);
        updateHeight(y->left);
        updateHeight(y);
        return y;
    }

    std::unique_ptr<AVLNode<T>> insertImpl(std::unique_ptr<AVLNode<T>> node, const T &v) {
        if (!node) return std::make_unique<AVLNode<T>>(v);
        if (cmp(v, node->value)) node->left = insertImpl(std::move(node->left), v);
        else node->right = insertImpl(std::move(node->right), v);

        updateHeight(node);
        int bf = balanceFactor(node);

        // Left Left
        if (bf > 1 && cmp(v, node->left->value)) return rotateRight(std::move(node));
        // Right Right
        if (bf < -1 && !cmp(v, node->right->value)) return rotateLeft(std::move(node));
        // Left Right
        if (bf > 1 && !cmp(v, node->left->value)) {
            node->left = rotateLeft(std::move(node->left));
            return rotateRight(std::move(node));
        }
        // Right Left
        if (bf < -1 && cmp(v, node->right->value)) {
            node->right = rotateRight(std::move(node->right));
            return rotateLeft(std::move(node));
        }
        return node;
    }

    void inorderImpl(const AVLNode<T> *n, std::vector<T> &out) const {
        if (!n) return;
        inorderImpl(n->left.get(), out);
        out.push_back(n->value);
        inorderImpl(n->right.get(), out);
    }
};
