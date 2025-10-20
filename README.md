# DS_Project

## Download SFML 
```bash
sudo pacman -S sfml
```
## Compile main.cpp from terminal
command to compile it
```bash
g++ main.cpp -o maze -lsfml-graphics -lsfml-window -lsfml-system
```

## Build with CMake
copiar el repo  
cd repo  
 Put the files into the structure shown earlier.  
 `mkdir build && cd build`  
 `cmake ..`  
 `cmake --build .`  
 Run `./maze_app`  


 Si hay errores, remover el oldMain.cpp, ya que CMake es sensible a diferentes main en el mismo folder
 
