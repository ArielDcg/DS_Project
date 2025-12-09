# DS_Project
## Video de Resultados
[Demostración](https://drive.google.com/file/d/1WFURZozSwdg8ouFHPSDQJrtCVzJ8VCXf/view?usp=sharing)

## Manual de Usuario
Para realizar el setup completo, dirígase al [manual de usuario](docs/manualDeUsuario.md)

### Compilación Rápida

#### Usando los scripts incluidos (Recomendado):

```bash
# Compilar el proyecto
./build.sh

# Ejecutar el programa
./run.sh
```

#### Compilación manual con CMake:

```bash
# Crear directorio de compilación
mkdir -p build
cd build

# Configurar con CMake
cmake ..

# Compilar
make

# Ejecutar
./maze_app
```

**Nota importante**: Todos los archivos generados por CMake se almacenan en el directorio `build/` para mantener el repositorio limpio. Nunca ejecutes CMake directamente en el directorio raíz o `src/`.

---

## Modos de Juego

El programa ofrece **6 modos** distintos de funcionamiento:

### 1. **Classic Mode**
Modo tradicional de generación y resolución de laberintos.
- **Inicio**: Centro del laberinto
- **Meta**: Esquina aleatoria
- Visualización paso a paso de la generación y resolución

### 2. **Collector Mode (3 Treasures)**
Modo desafiante donde el solver debe recolectar 3 tesoros antes de llegar a la meta.
- **Inicio**: Centro del laberinto
- **Objetivo**: Recolectar 3 tesoros + llegar a la meta
- **Estrategias disponibles**: A*, Greedy, UCS, DFS
- Visualización con colores diferentes para cada segmento del camino

### 3. **User vs Solver Mode**
Compite contra un algoritmo de solución en tiempo real.
- **Controles**: W/A/S/D para moverse
- **Objetivo**: Llegar a la meta antes que el solver
- **Dificultad ajustable**: Easy (solver lento), Medium, Hard (solver rápido)

### 4. **Origin Shift (Living Maze)** - NUEVO ⭐ (Grafo)
El laberinto **cambia en tiempo real** mientras el solver intenta resolverlo.

#### ¿Qué es Origin Shift?
El laberinto se representa como un **grafo dirigido** (árbol) donde cada celda apunta hacia una "raíz". Cuando la raíz se mueve, las conexiones del grafo cambian, modificando las paredes del laberinto.

#### Estructura de Datos: Lista de Adyacencia
```cpp
// parentMap[x][y] = dirección hacia el padre (0=Up, 1=Left, 2=Right, 3=Down)
// -1 significa que es una raíz
std::vector<std::vector<int>> parentMap;
```

Esta es una **lista de adyacencia implícita** donde cada nodo conoce su conexión con el padre.

#### Múltiples Raíces Simultáneas
Para aumentar la dificultad, se implementaron **3 raíces** que se mueven simultáneamente:

| Raíz | Color | Ubicación Inicial |
|------|-------|-------------------|
| 1 | 🟣 Magenta | Centro del mapa |
| 2 | 🔵 Cyan | Esquina superior-izquierda |
| 3 | 🟡 Amarillo | Esquina inferior-derecha |

#### Métricas del Grafo (mostradas en pantalla)
- **Nodes**: Total de nodos (celdas) = width × height
- **Edges**: Aristas (conexiones sin pared). En un árbol: edges ≈ nodes - 1
- **Active Roots**: Número de raíces modificando el grafo

#### ¿Por qué usar un Grafo?
1. **Representación eficiente**: Solo almacenamos la dirección del padre por celda
2. **Modificación O(1)**: Cambiar una arista es simplemente actualizar un valor en parentMap
3. **Inversión de aristas**: El algoritmo Origin Shift invierte aristas del grafo, demostrando operaciones típicas de grafos

### 5. **Algorithm Ranking (AVL Tree)**
Sistema de benchmarking que compara el rendimiento de los **algoritmos de solución (solvers)**.

#### ¿Qué hace?
Compara los **4 solvers** probándolos en laberintos generados por diferentes algoritmos:
- **4 solvers evaluados**: A*, Greedy, UCS, DFS
- **Cada solver se prueba en**: 4 tipos de laberintos × 30 iteraciones = **120 ejecuciones por solver**
- **Total**: 4 solvers × 120 ejecuciones = **480 ejecuciones**

El sistema garantiza que todos los solvers sean probados en los mismos tipos de laberintos (DFS, Prim's, Hunt&Kill, Kruskal's), eliminando el sesgo de que ciertos algoritmos funcionen mejor en ciertos tipos de laberintos.

#### ¿Por qué los valores cambian?
Los valores **NO** cambian entre compilaciones. Los valores pueden variar **ligeramente entre ejecuciones** porque:
1. Cada laberinto se genera **aleatoriamente**
2. Las posiciones de los tesoros son **aleatorias**
3. El sistema calcula el **promedio de 120 ejecuciones por solver** para obtener resultados estadísticamente sólidos

#### ¿Qué significan las columnas?

| Columna | Descripción | Interpretación |
|---------|-------------|----------------|
| **Rank** | Posición en el ranking | 1 = Mejor solver |
| **Solver Algorithm** | Algoritmo de solución evaluado | A*, Greedy, UCS o DFS |
| **Score** | Puntuación compuesta | **Mayor = Mejor**. Rango típico: 0-1000 |
| **Nodes** | Nodos expandidos promedio | **Menor = Más eficiente**. Cuenta celdas marcadas como CLOSED |
| **Time(ms)** | Tiempo de ejecución promedio | **Menor = Más rápido**. Solo mide el tiempo de resolución |
| **Treasures** | Tesoros recolectados promedio | **Mayor = Mejor** (máximo: 3) |

#### ¿Cómo se calcula el Score?
```
Score inicial = 1000 puntos

Penalizaciones:
- Nodos expandidos: -0.5 puntos por nodo (máx. -500)
- Tiempo de ejecución: -500 puntos por segundo (máx. -100)

Bonificaciones:
+ 50 puntos por cada tesoro recolectado (máx. +150)

Score final = Score inicial - Penalizaciones + Bonificaciones
```

**Ejemplo de interpretación:**
- Si A* está en el puesto 1 con Score 850, significa que A* es el solver más eficiente en promedio
- Si ves Nodes=150, significa que expandió 150 nodos durante la búsqueda
- Si Time=10ms, el solver resolvió el laberinto en 10 milisegundos en promedio

**Rangos de Score:**
- Scores > 700 = Excelente rendimiento (muy eficiente)
- Scores 500-700 = Buen rendimiento (eficiente)
- Scores 300-500 = Rendimiento regular
- Scores < 300 = Rendimiento bajo (ineficiente)

#### Información del Árbol AVL
En la parte inferior se muestra:
- **Solvers**: Número de solvers evaluados (siempre 4)
- **Height**: Altura del árbol AVL (típicamente 2-3 para 4 nodos)
- **Balanced**: Confirma que el árbol está balanceado (siempre "Yes")

#### ¿Por qué usar un Árbol AVL?
El Árbol AVL auto-balanceado mantiene los solvers ordenados por score de manera eficiente:
- **Inserción**: O(log n) - Rápido incluso con muchos solvers
- **Búsqueda**: O(log n) - Encontrar un solver específico
- **Ordenamiento**: In-order traversal da ranking ordenado
- **Auto-balanceo**: Garantiza eficiencia sin importar el orden de inserción

### 6. **Exploration Heatmap (Sparse Matrix)**
Visualización del comportamiento de exploración de los algoritmos de solución.

#### ¿Cómo acceder al Heatmap?
Hay **dos formas** de ver el Heatmap:

1. **Desde el menú principal**: Opción 4 - Genera un laberinto y lo resuelve con A* para mostrar el heatmap
2. **Después de cualquier ejecución del Collector Mode**: Presiona la tecla **H** cuando termine el solver para ver el heatmap de **ESA ejecución específica**

#### ¿Qué muestra?
Un mapa de calor que indica **cuántas veces se visitó cada celda** durante la búsqueda de camino del algoritmo que acabas de ejecutar.

#### Colores del Heatmap
- **Negro**: Celda nunca visitada
- **Azul oscuro**: Pocas visitas (baja exploración)
- **Verde**: Visitas moderadas
- **Amarillo**: Muchas visitas
- **Rojo/Naranja**: Visitada repetidamente (máxima exploración)

#### ¿Qué significan las estadísticas?

| Estadística | Descripción | Interpretación |
|-------------|-------------|----------------|
| **Total Visits** | Suma total de todas las visitas | Indica cuánto "trabajo" hizo el algoritmo |
| **Unique Cells** | Celdas únicas visitadas | Número de celdas exploradas al menos una vez |
| **Max Visits** | Máximo de visitas en una celda | Celdas problemáticas donde el algoritmo retrocedió mucho |
| **Avg Visits** | Promedio de visitas por celda | Eficiencia promedio de exploración |
| **Coverage** | Porcentaje del grid visitado | % del laberinto que se exploró |

#### Sparse Matrix (Matriz Dispersa)
El panel muestra la eficiencia de memoria de la matriz dispersa:
- **Size**: Número de entradas almacenadas (solo celdas visitadas)
- **vs Full**: Tamaño de una matriz completa (40×28 = 1120 celdas)
- **Saved**: Celdas NO almacenadas (ahorro de memoria)

**Ejemplo:**
- Size: 340 entries
- vs Full: 1120
- Saved: 780 (70% de ahorro de memoria)

Esto demuestra que la matriz dispersa solo almacena las celdas **realmente visitadas**, ahorrando memoria significativamente.

#### ¿Por qué algunos valores cambian?
Como el laberinto se genera aleatoriamente cada vez:
- El **número de visitas** puede variar según la complejidad del laberinto
- La **cobertura** depende de la estructura del laberinto generado
- Laberintos más complejos → más exploración → más visitas

#### Comparando Algoritmos
Una de las características más útiles es que puedes:
1. Generar un laberinto con un algoritmo de generación
2. Resolver con un algoritmo de solución (ej: A*)
3. Ver el heatmap presionando **H**
4. Cerrar la ventana del heatmap
5. Cerrar la ventana de ejecución
6. Repetir con **otro algoritmo de solución** en el **mismo tipo de laberinto**
7. Comparar visualmente cómo diferentes algoritmos exploran el espacio

**Ejemplo de uso:**
- Genera con DFS, resuelve con A*, presiona H → ves cómo A* explora
- Genera con DFS, resuelve con Greedy, presiona H → ves cómo Greedy explora
- Compara: ¿Cuál visitó menos celdas? ¿Cuál fue más eficiente?

---

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
| **Origin Shift** | **Lista Adyacencia (parentMap)** | **Representación de grafo dirigido** |
| Algorithm Ranking | AVL Tree               | Ranking auto-balanceado de solvers       |
| Exploration Heatmap | Sparse Matrix        | Almacenamiento eficiente de visitas      |

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
