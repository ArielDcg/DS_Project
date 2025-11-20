#include "AlgorithmRankingAVL.h"
#include <algorithm>
#include <cmath>

float AlgorithmStats::calculateScore() const {
    // Score compuesto (mayor = mejor)
    // Penalizar nodos expandidos y longitud de camino
    // Premiar velocidad y tesoros
    
    float score = 1000.0f;
    
    // Penalizar expansión (0-500 puntos perdidos)
    if (nodesExpanded > 0) {
        score -= std::min(500.0f, nodesExpanded * 0.5f);
    }
    
    // Penalizar longitud (0-300 puntos perdidos)
    if (pathLength > 0) {
        score -= std::min(300.0f, pathLength * 2.0f);
    }
    
    // Penalizar tiempo (0-100 puntos perdidos)
    if (executionTime > 0.0f) {
        score -= std::min(100.0f, executionTime * 500.0f);
    }
    
    // Bonus por tesoros (0-150 puntos)
    score += treasuresCollected * 50.0f;
    
    return std::max(0.0f, score);
}

std::string AlgorithmStats::getKey() const {
    return algorithmName + "_" + generationAlgo;
}


AlgorithmRankingAVL::AlgorithmRankingAVL() 
    : root(nullptr), nodeCount(0) {}

AlgorithmRankingAVL::~AlgorithmRankingAVL() {
    destroyTree(root);
}

void AlgorithmRankingAVL::destroyTree(AVLNode* node) {
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

void AlgorithmRankingAVL::clear() {
    destroyTree(root);
    root = nullptr;
    nodeCount = 0;
}


int AlgorithmRankingAVL::height(AVLNode* node) const {
    return node ? node->height : 0;
}

int AlgorithmRankingAVL::getBalance(AVLNode* node) const {
    return node ? height(node->left) - height(node->right) : 0;
}

void AlgorithmRankingAVL::updateHeight(AVLNode* node) {
    if (node) {
        node->height = 1 + std::max(height(node->left), height(node->right));
    }
}

int AlgorithmRankingAVL::size() const {
    return nodeCount;
}

int AlgorithmRankingAVL::getHeight() const {
    return height(root);
}


AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::rotateRight(AVLNode* y) {

    AVLNode* x = y->left;
    AVLNode* B = x->right;
    
    // Rotar
    x->right = y;
    y->left = B;
    
    // Actualizar alturas
    updateHeight(y);
    updateHeight(x);
    
    return x;
}

AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::rotateLeft(AVLNode* x) {

    AVLNode* y = x->right;
    AVLNode* B = y->left;
    
    // Rotar
    y->left = x;
    x->right = B;
    
    // Actualizar alturas
    updateHeight(x);
    updateHeight(y);
    
    return y;
}

AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::rotateLeftRight(AVLNode* node) {
    
    node->left = rotateLeft(node->left);
    return rotateRight(node);
}

AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::rotateRightLeft(AVLNode* node) {

    node->right = rotateRight(node->right);
    return rotateLeft(node);
}


AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::balance(AVLNode* node) {
    if (!node) return nullptr;
    
    updateHeight(node);
    int balanceFactor = getBalance(node);
    
    // Caso 1: Desbalance Izquierdo-Izquierdo (LL)
    if (balanceFactor > 1 && getBalance(node->left) >= 0) {
        return rotateRight(node);
    }
    
    // Caso 2: Desbalance Derecho-Derecho (RR)
    if (balanceFactor < -1 && getBalance(node->right) <= 0) {
        return rotateLeft(node);
    }
    
    // Caso 3: Desbalance Izquierdo-Derecho (LR)
    if (balanceFactor > 1 && getBalance(node->left) < 0) {
        return rotateLeftRight(node);
    }
    
    // Caso 4: Desbalance Derecho-Izquierdo (RL)
    if (balanceFactor < -1 && getBalance(node->right) > 0) {
        return rotateRightLeft(node);
    }

    // Ya está balanceado
    return node;
}


void AlgorithmRankingAVL::insert(const AlgorithmStats& stats) {
    root = insertNode(root, stats);
}

AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::insertNode(
    AVLNode* node, 
    const AlgorithmStats& stats
) {
    // Caso base: insertar aquí
    if (!node) {
        nodeCount++;
        return new AVLNode(stats);
    }
    
    // Ordenar por score (DESCENDENTE: mayor score = mejor)
    // Pero si existe, actualizar
    std::string key = stats.getKey();
    std::string nodeKey = node->data.getKey();
    
    if (key == nodeKey) {
        // Actualizar nodo existente
        node->data = stats;
        node->score = stats.calculateScore();
        return node;
    }
    
    // Comparar por score (mayor a la izquierda)
    if (stats.calculateScore() > node->score) {
        node->left = insertNode(node->left, stats);
    } else {
        node->right = insertNode(node->right, stats);
    }
    
    // Balancear después de inserción
    return balance(node);
}


bool AlgorithmRankingAVL::find(const std::string& key, AlgorithmStats& out) const {
    AVLNode* result = findNode(root, key);
    if (result) {
        out = result->data;
        return true;
    }
    return false;
}

AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::findNode(
    AVLNode* node, 
    const std::string& key
) const {
    if (!node) return nullptr;
    
    std::string nodeKey = node->data.getKey();
    
    if (key == nodeKey) {
        return node;
    }
    
    // Buscar en ambos subárboles (no sabemos si está por score)
    AVLNode* leftResult = findNode(node->left, key);
    if (leftResult) return leftResult;
    
    return findNode(node->right, key);
}


void AlgorithmRankingAVL::update(const std::string& key, const AlgorithmStats& newStats) {
    // Buscar y eliminar viejo
    root = deleteNode(root, key);
    
    // Insertar nuevo
    insert(newStats);
}

AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::deleteNode(
    AVLNode* node, 
    const std::string& key
) {
    if (!node) return nullptr;
    
    std::string nodeKey = node->data.getKey();
    
    if (key == nodeKey) {
        // Caso 1: Nodo hoja
        if (!node->left && !node->right) {
            delete node;
            nodeCount--;
            return nullptr;
        }
        
        // Caso 2: Un hijo
        if (!node->left) {
            AVLNode* temp = node->right;
            delete node;
            nodeCount--;
            return temp;
        }
        if (!node->right) {
            AVLNode* temp = node->left;
            delete node;
            nodeCount--;
            return temp;
        }
        
        // Caso 3: Dos hijos - reemplazar con sucesor
        AVLNode* successor = findMin(node->right);
        node->data = successor->data;
        node->score = successor->score;
        node->right = deleteNode(node->right, successor->data.getKey());
    } else {
        // Buscar en ambos subárboles
        node->left = deleteNode(node->left, key);
        node->right = deleteNode(node->right, key);
    }
    
    return balance(node);
}

AlgorithmRankingAVL::AVLNode* AlgorithmRankingAVL::findMin(AVLNode* node) const {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

void AlgorithmRankingAVL::inorderTraversal(AVLNode* node, std::vector<AVLNode*>& result) const {
    if (!node) return;
    
    // Inorder: izquierda -> raíz -> derecha
    // Como insertamos mayor a la izquierda, esto da orden DESCENDENTE
    inorderTraversal(node->left, result);
    result.push_back(node);
    inorderTraversal(node->right, result);
}

std::vector<RankEntry> AlgorithmRankingAVL::getTopK(int k) const {
    std::vector<AVLNode*> allNodes;
    inorderTraversal(root, allNodes);
    
    std::vector<RankEntry> rankings;
    rankings.reserve(std::min(k, static_cast<int>(allNodes.size())));
    
    for (int i = 0; i < std::min(k, static_cast<int>(allNodes.size())); ++i) {
        rankings.emplace_back(allNodes[i]->data, i + 1);
    }
    
    return rankings;
}

std::vector<RankEntry> AlgorithmRankingAVL::getAll() const {
    std::vector<AVLNode*> allNodes;
    inorderTraversal(root, allNodes);
    
    std::vector<RankEntry> rankings;
    rankings.reserve(allNodes.size());
    
    for (size_t i = 0; i < allNodes.size(); ++i) {
        rankings.emplace_back(allNodes[i]->data, static_cast<int>(i) + 1);
    }
    
    return rankings;
}

bool AlgorithmRankingAVL::isBalanced() const {
    return isBalancedHelper(root);
}

bool AlgorithmRankingAVL::isBalancedHelper(AVLNode* node) const {
    if (!node) return true;
    
    int balance = getBalance(node);
    if (std::abs(balance) > 1) return false;
    
    return isBalancedHelper(node->left) && isBalancedHelper(node->right);
}