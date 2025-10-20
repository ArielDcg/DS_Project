# Setup en Windows

1. Clonar el repo
2. Descargar Msys
3. Abrir Msys MINGW64
4. Instalar los paquetes con estos comandos
```bash
   pacman -S --needed base-devel mingw-w64-x86_64-toolchain
   pacman -S mingw-w64-x86_64-sfml
   pacman -S cmake

```
5. comprobar correcta instalacion
```bash
  g++ --version
  cmake --version
```
6. Dirigirse al folder del proyecto
7. Ejecutar estos comandos
```bash
  cd /c/ruta/a/mi_proyecto
  mkdir build && cd build
  cmake ..
  cmake --build .
```
8. Correr el ejecutable
```bash
  ./maze_app
```
