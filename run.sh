#!/bin/bash
# Script para ejecutar MazeVisualizer
# Compila automáticamente si es necesario

# Colores para output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Verificar si existe el ejecutable
if [ ! -f "build/maze_app" ]; then
    echo -e "${YELLOW}No se encontró el ejecutable. Compilando...${NC}"
    ./build.sh

    if [ $? -ne 0 ]; then
        echo -e "${RED}Error: No se pudo compilar el proyecto.${NC}"
        exit 1
    fi
fi

# Ejecutar el programa
echo -e "${GREEN}=== Iniciando MazeVisualizer ===${NC}"
cd build && ./maze_app
