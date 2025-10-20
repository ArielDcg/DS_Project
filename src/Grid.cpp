#include "Grid.h"
#include <cstdlib>

Grid::Grid(int width, int height) : m_width(width), m_height(height) {
    m_grid.resize(m_height, std::vector<Cell>(m_width));
}

int Grid::width() const { return m_width; }
int Grid::height() const { return m_height; }

Cell &Grid::at(int x, int y) { return m_grid[y][x]; }
const Cell &Grid::at(int x, int y) const { return m_grid[y][x]; }

bool Grid::removeWall(int row, int column, short wall) {
    if (row < 0 || column < 0 || row >= m_height || column >= m_width) return false;
    if (wall == 0 && row == 0) return false;
    if (wall == 1 && column == 0) return false;
    if (wall == 2 && column == m_width - 1) return false;
    if (wall == 3 && row == m_height - 1) return false;

    m_grid[row][column].walls[wall] = false;
    if (wall == 0) m_grid[row - 1][column].walls[3] = false;
    else if (wall == 1) m_grid[row][column - 1].walls[2] = false;
    else if (wall == 2) m_grid[row][column + 1].walls[1] = false;
    else if (wall == 3) m_grid[row + 1][column].walls[0] = false;
    return true;
}

int Grid::pickRandomNeighborDir(int x, int y) const {
    std::vector<int> candidates;
    if (y > 0 && !m_grid[y - 1][x].visited) candidates.push_back(0);           // Up
    if (y < m_height - 1 && !m_grid[y + 1][x].visited) candidates.push_back(3); // Down
    if (x > 0 && !m_grid[y][x - 1].visited) candidates.push_back(1);           // Left
    if (x < m_width - 1 && !m_grid[y][x + 1].visited) candidates.push_back(2); // Right
    if (candidates.empty()) return -1;
    return candidates[std::rand() % candidates.size()];
}