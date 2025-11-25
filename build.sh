#!/bin/bash
# Script de compilación para MazeVisualizer
# Este script configura y compila el proyecto usando CMake

# Colores para output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Configurando MazeVisualizer ===${NC}"

# Crear directorio de compilación si no existe
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Creando directorio build...${NC}"
    mkdir -p build
fi

# Entrar al directorio de compilación
cd build

# Configurar con CMake
echo -e "${YELLOW}Ejecutando CMake...${NC}"
cmake ..

# Verificar si CMake tuvo éxito
if [ $? -ne 0 ]; then
    echo -e "${RED}Error: CMake falló. Verifica que todas las dependencias estén instaladas.${NC}"
    exit 1
fi

# Compilar
echo -e "${YELLOW}Compilando...${NC}"
make

# Verificar si la compilación tuvo éxito
if [ $? -ne 0 ]; then
    echo -e "${RED}Error: La compilación falló.${NC}"
    exit 1
fi

echo -e "${GREEN}=== Compilación exitosa ===${NC}"
echo -e "${YELLOW}Para ejecutar el programa, usa:${NC}"
echo -e "  cd build && ./maze_app"
echo -e "${YELLOW}O simplemente ejecuta:${NC}"
echo -e "  ./run.sh"
