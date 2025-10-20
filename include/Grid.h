#pragma once
#include <vector>

struct Cell {
    bool visited = false;
    // walls: 0 = Up, 1 = Left, 2 = Right, 3 = Down
    bool walls[4] = { true, true, true, true };
};

struct Coord {
    int x = 0;
    int y = 0;
    Coord(int xx = 0, int yy = 0) : x(xx), y(yy) {}
};

class Grid {
public:
    Grid(int width, int height);
    int width() const;
    int height() const;
    Cell &at(int x, int y);
    const Cell &at(int x, int y) const;

    // removeWall expects (row=y, column=x, wall)
    // wall: 0=Up,1=Left,2=Right,3=Down
    bool removeWall(int row, int column, short wall);
    // returns a random neighbor direction or -1 if none
    int pickRandomNeighborDir(int x, int y) const;

private:
    int m_width;
    int m_height;
    std::vector<std::vector<Cell>> m_grid; // indexed as m_grid[row][col] == m_grid[y][x]
};