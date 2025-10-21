# ¿Qué es SFML? (breve)

SFML (Simple and Fast Multimedia Library) es una librería en C++ para multimedia y desarrollo de aplicaciones gráficas 2D —ventanas, gráficos, audio, entrada (teclado/ratón/joystick), temporizadores y redes— con una API simple y orientada a objetos. Es ideal para prototipado, juegos 2D y aplicaciones interactivas donde quieres control sin la complejidad de APIs más bajas.

# Instalación — paso a paso (breve)

A continuación pasos típicos para **Windows, Linux y macOS**, y un ejemplo rápido con **CMake**.

---

## Windows (MinGW-w64 o MSVC)

1. Ve a la página de SFML y descarga la versión correspondiente a tu compilador (p. ej. *SFML 2.x - MinGW* o *MSVC*).
2. Extrae el ZIP en `C:\libs\SFML` (o donde prefieras).
3. Si usas **Visual Studio (MSVC)**:

   * Abre tu proyecto, añade la carpeta `include` en las propiedades de C/C++ → Additional Include Directories.
   * Añade los `.lib` (en `lib`) en Linker → Input → Additional Dependencies.
   * Copia los `.dll` necesarios (en `bin`) al directorio del ejecutable o al PATH.
4. Si usas **MinGW/g++**:

   * Añade `-IC:\libs\SFML\include` y linkea con `-LC:\libs\SFML\lib -lsfml-graphics -lsfml-window -lsfml-system` (y otras según necesites).
   * Ejemplo:

     ```bash
     g++ main.cpp -IC:/libs/SFML/include -LC:/libs/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system -o juego.exe
     ```
   * Copia los DLLs de `bin` junto al `.exe`.

---

## Ubuntu / Debian (apt)

1. Actualiza e instala:

   ```bash
   sudo apt update
   sudo apt install libsfml-dev
   ```
2. Compilar:

   ```bash
   g++ main.cpp -lsfml-graphics -lsfml-window -lsfml-system -o juego
   ```

---

## Arch Linux / Manjaro (pacman)

```bash
sudo pacman -Syu
sudo pacman -S sfml
```

y compilar igual que arriba.

---

## macOS (Homebrew)

```bash
brew update
brew install sfml
```

Compilar:

```bash
g++ main.cpp -lsfml-graphics -lsfml-window -lsfml-system -o juego
```

(o usa `clang++` según tu configuración).

---

