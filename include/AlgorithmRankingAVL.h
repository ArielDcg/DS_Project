#pragma once
#include <string>
#include <vector>
#include <memory>


// Métricas de un algoritmo
struct AlgorithmStats {
    std::string algorithmName;   // "A*", "Greedy", "UCS", "DFS"
    std::string generationAlgo;  // "DFS", "Prim's", etc.
    int nodesExpanded = 0;       // Nodos explorados
    int pathLength = 0;          // Longitud del camino final
    float executionTime = 0.0f;  // Tiempo en segundos
    int treasuresCollected = 0;  // Tesoros recolectados
    
    // Puntuacion compuesta (mayor = mejor)
    float calculateScore() const;
    
    // Clave única: algorithmName + generationAlgo
    std::string getKey() const;
};

// Entrada del ranking (para retornar)
struct RankEntry {
    AlgorithmStats stats;
    int rank = 0;  // 1 = mejor
    
    RankEntry(const AlgorithmStats& s, int r) : stats(s), rank(r) {}
};

class AlgorithmRankingAVL {
public:
    AlgorithmRankingAVL();
    ~AlgorithmRankingAVL();
    
    // ===================================
    // OPERACIONES PRINCIPALES
    // ===================================
    
    // Insertar/actualizar algoritmo
    void insert(const AlgorithmStats& stats);
    void update(const std::string& key, const AlgorithmStats& newStats);
    
    // Obtener top K algoritmos (ordenados por score descendente)
    std::vector<RankEntry> getTopK(int k) const;
    
    // Obtener todos los algoritmos ordenados
    std::vector<RankEntry> getAll() const;
    
    // Buscar un algoritmo específico
    bool find(const std::string& key, AlgorithmStats& out) const;
    
    // Número de algoritmos en el árbol
    int size() const;
    
    // Limpiar árbol
    void clear();
    
    // Verificar si el árbol está balanceado (debug)
    bool isBalanced() const;
    
    // Obtener altura del árbol
    int getHeight() const;

private:
    // ===================================
    // NODO DEL ÁRBOL AVL
    // ===================================
    struct AVLNode {
        AlgorithmStats data;
        float score;  // Cache del score para comparaciones
        
        AVLNode* left;
        AVLNode* right;
        int height;
        
        AVLNode(const AlgorithmStats& stats) 
            : data(stats), 
              score(stats.calculateScore()),
              left(nullptr), 
              right(nullptr), 
              height(1) {}
    };
    
    AVLNode* root;
    int nodeCount;
    
    // ===================================
    // FUNCIONES AUXILIARES
    // ===================================
    
    // Altura de un nodo
    int height(AVLNode* node) const;
    
    // Factor de balance
    int getBalance(AVLNode* node) const;
    
    // Actualizar altura
    void updateHeight(AVLNode* node);
    
    // ===================================
    // ROTACIONES (AUTO-BALANCEO)
    // ===================================
    
    // Rotación simple derecha (LL)
    AVLNode* rotateRight(AVLNode* y);
    
    // Rotación simple izquierda (RR)
    AVLNode* rotateLeft(AVLNode* x);
    
    // Rotación doble izquierda-derecha (LR)
    AVLNode* rotateLeftRight(AVLNode* node);
    
    // Rotación doble derecha-izquierda (RL)
    AVLNode* rotateRightLeft(AVLNode* node);
    
    // ===================================
    // OPERACIONES RECURSIVAS
    // ===================================
    
    AVLNode* insertNode(AVLNode* node, const AlgorithmStats& stats);
    AVLNode* deleteNode(AVLNode* node, const std::string& key);
    AVLNode* findMin(AVLNode* node) const;
    AVLNode* balance(AVLNode* node);
    
    void inorderTraversal(AVLNode* node, std::vector<AVLNode*>& result) const;
    void destroyTree(AVLNode* node);
    
    AVLNode* findNode(AVLNode* node, const std::string& key) const;
    
    bool isBalancedHelper(AVLNode* node) const;
};