# DS_Project
## Manual de Usuario
Para realizar el setup, dirígase al manual [manual](docs/manualDeUsuario.md)

## Generación de Laberintos Perfectos - Documentación 

### Descripción del Proyecto
Este proyecto implementa y compara diferentes algoritmos clásicos para la generación de laberintos perfectos - laberintos sin bucles y con una única solución posible entre dos puntos.

Los algoritmos implementados son:
- **Depth-First Search (DFS)**
- **Prim's Algorithm** 
- **Hunt-and-Kill**
- **Kruskal's Algorithm**
- **DFS Solver** (para resolución)

---

### Estructuras de Datos Implementadas

#### 1. Pila (Stack) - Depth-First Search

##### Implementación:
```cpp
std::stack<Cell> stack;
```

##### Funcionamiento:
1. Selección de celda inicial aleatoria
2. Marcado como visitada y apilado
3. Mientras la pila no esté vacía:
   - Toma de celda del tope
   - Búsqueda de vecino no visitado
   - Eliminación de pared entre celdas
   - Marcado y apilado del nuevo vecino
   - Retroceso (pop) si no hay vecinos disponibles

##### Propósito:
Permitir recorrido en profundidad con backtracking, generando un camino único entre celdas.

---

#### 2. Vector de Fronteras - Prim's Algorithm

##### Implementación:
```cpp
struct Frontier { 
    int cx, cy, dir; 
};
std::vector<Frontier> frontier;
```

##### Funcionamiento:
1. Elección de celda inicial aleatoria
2. Adición de celdas vecinas al vector de fronteras
3. Procesamiento mientras existan fronteras:
   - Selección aleatoria de frontera
   - Verificación de celda no visitada
   - Eliminación de pared y marcado
   - Adición de nuevas fronteras
   - Eliminación de frontera procesada

##### Propósito:
Gestión dinámica de fronteras para expansión uniforme del laberinto mediante selección aleatoria.

---

#### 3. Cola Circular - Hunt-and-Kill

##### Implementación:
```cpp
class CircularQueue {
private:
    int front, rear, size;
    std::vector<Cell> data;
public:
    // Implementación de operaciones básicas
    bool isEmpty() const;
    bool isFull() const;
    void enqueue(const Cell& c);
    Cell dequeue();
};
```

##### Funcionamiento:
- **Fase Kill**: Recorrido aleatorio por celdas no visitadas
- **Fase Hunt**: Búsqueda secuencial eficiente usando cola circular
- Reinicio de recorrido al encontrar celda válida

##### Propósito:
Optimización de búsquedas secuenciales durante la fase de caza mediante recorrido circular continuo.

---

#### 4. Cola de Prioridad - Kruskal's Algorithm

##### Implementación:
```cpp
struct Edge {
    int x1, y1, x2, y2;
    int weight;
    bool operator<(const Edge& other) const {
        return weight > other.weight;
    }
};

std::priority_queue<Edge> edges;
```

##### Funcionamiento:
1. Generación de todas las paredes posibles con pesos aleatorios
2. Inserción en cola de prioridad
3. Procesamiento por peso ascendente:
   - Verificación de conjuntos disjuntos (Union-Find)
   - Eliminación de pared y unión de conjuntos
4. Continuación hasta conexión completa

##### Propósito:
Garantizar conexiones sin ciclos mediante selección óptima de paredes basada en peso.

---

#### 5. Pila (Stack) - DFS Solver

##### Funcionamiento:
1. Inicio desde celda de entrada
2. Apilado de movimientos posibles
3. Exploración en profundidad
4. Retroceso (backtracking) en callejones sin salida
5. Terminación al alcanzar celda de salida

##### Propósito:
Verificación de la propiedad de laberinto perfecto mediante búsqueda de camino único.

---

### Resumen de Estructuras de Datos

| Algoritmo       | Estructura Principal     | Propósito Clave                          |
|-----------------|--------------------------|------------------------------------------|
| DFS             | Stack                    | Backtracking y recorrido profundidad     |
| Prim's          | Vector (Frontier)        | Expansión aleatoria desde visitadas      |
| Hunt-and-Kill   | Circular Queue           | Búsqueda secuencial eficiente            |
| Kruskal         | Priority Queue           | Selección óptima de paredes por peso     |
| DFS Solver      | Stack                    | Búsqueda de solución única               |

---

### Análisis de Complejidad

#### DFS Generation

|Algoritmo|Complejidad Temporal|Complejidad Espacial|Característica|
|---|---|---|---|
|**Hunt-and-Kill**|O(n²) peor caso|O(1) adicional|Búsqueda secuencial costosa|
|DFS|O(n)|O(n) stack|Backtracking eficiente|
|Prim's|O(n log n)|O(n) frontier|Manejo de fronteras|
|Kruskal|O(n log n)|O(n) Union-Find|Procesamiento por pesos|

---

### Características de Output

Cada algoritmo produce laberintos con propiedades distintivas:

- **DFS**: Sesgo hacia caminos largos y pasillos retorcidos
- **Prim's**: Distribución más uniforme y aleatoria
- **Kruskal**: Mayor número de pasillos cortos y dead-ends
- **Hunt-and-Kill**: Balance entre aleatoriedad y estructura

---

### Conclusión

La selección de estructuras de datos es crítica para el rendimiento y características del laberinto generado:

- Las estructuras **LIFO** (Stack) optimizan backtracking
- Las estructuras **FIFO** (Queue) facilitan búsquedas ordenadas  
- Las **colas de prioridad** permiten selección óptima
- Los **vectores dinámicos** ofrecen flexibilidad en manejo de fronteras

Esta implementación demuestra cómo estructuras de datos adecuadas garantizan eficiencia y propiedades topológicas correctas en la generación de laberintos perfectos.
