## Setup en Windows
[MsysQuickSetup](windows.md)

## Setup Linux
## Download SFML 
Para poder utilizar la interfaz grafica es necesario instalar SFML, como se especifica a continuacion.[SFML](SFML_INSTALLATION.md)
Se recomienda utilizar la última versión 3.0

## Compile main.cpp en terminal
Comando con g++
```bash
g++ main.cpp -o maze -lsfml-graphics -lsfml-window -lsfml-system
```

## Build with CMake
copiar el repo  
cd repo  
```bash
  mkdir build && cd build  
  cmake ..
  cmake --build .  
 ./maze_app
```  


 Si hay errores, remover el oldMain.cpp, ya que CMake es sensible a diferentes main en el mismo folder
