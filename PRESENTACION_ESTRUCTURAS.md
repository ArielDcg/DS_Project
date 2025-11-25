# Presentación: Estructuras de Datos en MazeVisualizer
## Guía para Presentación de 6 Minutos

---

## 1. Introducción (30 segundos)
**Proyecto:** Sistema de visualización de laberintos con algoritmos de generación y solución
**Objetivo:** Implementar estructuras de datos avanzadas para optimizar rendimiento y análisis

---

## 2. Árbol AVL - Sistema de Ranking (2 minutos)

### ¿Qué es?
Árbol binario de búsqueda auto-balanceado que mantiene altura O(log n)

### ¿Dónde se usa?
**Archivo:** `src/AlgorithmRankingAVL.cpp` y `include/AlgorithmRankingAVL.h`

### ¿Para qué?
Mantener un **ranking ordenado** de los algoritmos de solución (A*, Greedy, UCS, DFS) basado en su rendimiento.

### Implementación
```cpp
struct AVLNode {
    AlgorithmStats data;      // Estadísticas del algoritmo
    float score;              // Score calculado (cache)
    AVLNode* left;
    AVLNode* right;
    int height;               // Altura para balanceo
};
```

### Operaciones clave
- **Inserción con auto-balanceo:** O(log n)
  - Rotaciones simples: LL, RR
  - Rotaciones dobles: LR, RL
- **Obtener Top K:** O(K + log n) mediante recorrido in-order
- **Búsqueda:** O(log n)

### Ventajas en el proyecto
✅ **Ordenamiento automático** por score (mejor a peor)
✅ **Actualizaciones eficientes** cuando se ejecutan nuevos benchmarks
✅ **Balance garantizado** - sin degradación a O(n) como BST normal
✅ **Consultas rápidas** de rankings en tiempo real

### Ejemplo de uso
```cpp
AlgorithmRankingAVL ranking;

// Insertar resultados de 4 solvers × 120 pruebas cada uno
for (cada solver) {
    AlgorithmStats stats = runBenchmark(solver);
    ranking.insert(stats);  // Auto-balanceo y ordenamiento
}

// Obtener top 4
auto top4 = ranking.getTopK(4);  // O(4 + log 4) = O(1)
```

---

## 3. Matriz Dispersa (Sparse Matrix) - Heatmap de Exploración (1.5 minutos)

### ¿Qué es?
Matriz que solo almacena valores no-cero usando una tabla hash

### ¿Dónde se usa?
**Archivo:** `src/ExplorationHeatmap.cpp` y `include/ExplorationHeatmap.h`

### ¿Para qué?
Registrar y visualizar qué celdas visitó cada algoritmo durante la búsqueda, optimizando memoria.

### Implementación
```cpp
class ExplorationHeatmap {
private:
    int width, height;
    // Solo almacena celdas visitadas (no toda la matriz)
    std::unordered_map<Coord, int, CoordHash> visitCounts;
};
```

### ¿Por qué matriz dispersa?
En un laberinto de 40×28 = **1,120 celdas**:
- **Matriz densa:** 1,120 × 4 bytes = 4,480 bytes (siempre)
- **Matriz dispersa:** Solo celdas visitadas × (8 + 4) bytes
  - A* visita ~150 celdas → 1,800 bytes (60% ahorro)
  - DFS visita ~800 celdas → 9,600 bytes (usa más pero crece dinámicamente)

### Operaciones
- **Registrar visita:** `recordVisit(x, y)` → O(1) promedio
- **Consultar visitas:** `getVisitCount(x, y)` → O(1) promedio
- **Normalizar para visualización:** `toNormalizedGrid()` → O(celdas visitadas)

### Ventajas
✅ **Ahorro de memoria** en algoritmos eficientes (A*, Greedy)
✅ **Inserción dinámica** - crece según necesidad
✅ **Acceso rápido** O(1) con hash
✅ **Estadísticas fáciles** - número de celdas únicas visitadas

---

## 4. Cola de Prioridad (Priority Queue) - Algoritmos de Búsqueda (1.5 minutos)

### ¿Qué es?
Heap binario (min-heap) que mantiene el elemento mínimo en la raíz

### ¿Dónde se usa?
**Archivo:** `src/CollectorSolver.cpp` - Funciones `stepAStar()`, `stepGreedy()`, `stepUCS()`

### ¿Para qué?
Seleccionar eficientemente la celda con **menor costo** (UCS), **menor heurística** (Greedy) o **menor f = g + h** (A*)

### Implementación
```cpp
// En stepAStar()
struct Node {
    Coord c;
    float f;  // f = g + h
    bool operator>(const Node& other) const {
        return f > other.f;  // Min-heap
    }
};

std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
```

### Comparación de algoritmos

| Algoritmo | Criterio de prioridad | Complejidad |
|-----------|----------------------|-------------|
| **A*** | f = g(n) + h(n) | O((V+E) log V) |
| **Greedy** | h(n) (solo heurística) | O((V+E) log V) |
| **UCS** | g(n) (solo costo) | O((V+E) log V) |
| **DFS** | Stack (LIFO) - sin prioridad | O(V+E) |

### Ventajas
✅ **Extracción del mínimo en O(log n)** vs O(n) con búsqueda lineal
✅ **Inserción eficiente O(log n)**
✅ **Garantía de optimalidad** en A* y UCS

---

## 5. Tablas Hash (Hash Maps) - Estados y Costos (1 minuto)

### ¿Dónde se usa?
**Archivo:** `src/CollectorSolver.cpp`

### Usos múltiples

#### 5.1 Conjunto de Celdas Cerradas (CLOSED)
```cpp
std::unordered_map<Coord, bool, CoordHash> closed;
```
**Para qué:** Evitar re-explorar celdas ya visitadas → O(1) consulta

#### 5.2 Registro de Costos (g-score)
```cpp
std::vector<std::vector<float>> gScore;  // Matriz densa por acceso frecuente
```
**Para qué:** Almacenar costo acumulado desde inicio a cada celda

#### 5.3 Registro de Procedencia (came-from)
```cpp
std::unordered_map<Coord, Coord, CoordHash> cameFrom;
```
**Para qué:** Reconstruir camino óptimo desde meta hasta inicio → O(longitud camino)

### Ventajas
✅ **Acceso O(1) promedio** para consultas y actualizaciones
✅ **Función hash personalizada** para coordenadas
✅ **Prevención de ciclos infinitos** en búsqueda

---

## 6. Grafo Implícito - Representación del Laberinto (30 segundos)

### ¿Qué es?
El laberinto se modela como un **grafo no dirigido** donde:
- **Vértices (V):** Celdas transitables
- **Aristas (E):** Conexiones entre celdas adyacentes (↑↓←→)

### Implementación
No se almacena explícitamente - se calcula on-the-fly:
```cpp
// Obtener vecinos de una celda
std::vector<Coord> neighbors = g.getNeighbors(current);
```

### Ventajas
✅ **Ahorro de memoria** - no almacena matriz de adyacencia
✅ **Generación dinámica** - los muros cambian durante generación
✅ **Aplicable a todos los algoritmos** de búsqueda

---

## 7. Resumen de Complejidades

| Estructura | Operación | Complejidad | Uso en el proyecto |
|------------|-----------|-------------|-------------------|
| **AVL Tree** | Inserción/Búsqueda | O(log n) | Ranking de algoritmos |
| **Sparse Matrix** | Acceso/Inserción | O(1) promedio | Heatmap de exploración |
| **Priority Queue** | Extracción mínimo | O(log n) | A*, UCS, Greedy |
| **Hash Map** | Búsqueda/Inserción | O(1) promedio | Estados, costos, procedencia |
| **Grafo Implícito** | Obtener vecinos | O(4) = O(1) | Representación del maze |

---

## 8. Conclusión (30 segundos)

### Logros
✅ **Eficiencia:** Estructuras optimizadas para operaciones críticas
✅ **Escalabilidad:** O(log n) y O(1) en operaciones frecuentes
✅ **Memoria:** Sparse matrix ahorra hasta 60% en casos eficientes
✅ **Análisis:** AVL permite comparar 4 solvers × 120 pruebas = 480 ejecuciones en tiempo real

### Impacto
El uso correcto de estas estructuras permite:
- **Benchmarking rápido** (480 ejecuciones en segundos)
- **Visualización en tiempo real** sin lag
- **Análisis estadístico** preciso de algoritmos

---

## Archivos Clave para Demostración

1. **AVL Tree:** `include/AlgorithmRankingAVL.h` (líneas 68-82 - estructura del nodo)
2. **Sparse Matrix:** `include/ExplorationHeatmap.h` (línea 44 - unordered_map)
3. **Priority Queue:** `src/CollectorSolver.cpp` (búsqueda por "priority_queue")
4. **Hash Maps:** `src/CollectorSolver.cpp` (búsqueda por "unordered_map")
5. **Grafo:** `src/Grid.cpp` - función `getNeighbors()`

---

## Datos para Mostrar

### Tamaño del Laberinto
- **Dimensiones:** 40 × 28 = 1,120 celdas
- **Modificable en:** `src/main.cpp` líneas 36-37

### Benchmark del Ranking
- **4 solvers:** A*, Greedy, UCS, DFS
- **120 pruebas por solver** (30 iteraciones × 4 tipos de maze)
- **Total:** 480 ejecuciones

### Resultados típicos (ejemplo)
```
Rank  Solver      Score  Nodes  Time(ms)  Treasures
1.    A*          850    150    8         3
2.    Greedy      720    170    6         2
3.    UCS         680    200    12        3
4.    DFS         420    800    15        2
```

**Interpretación:**
- A* explora menos nodos (150 vs 800 de DFS)
- Score = 1000 - (nodos×0.5) - (tiempo×500) + (tesoros×50)
- AVL mantiene este ranking ordenado automáticamente
