#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

using namespace std;

class Cell {
public:
    //creates an array of bools/bits that determines the state of each wall of the cell
    // Up, Left, Right, Down
    bool walls[4] = {1, 1, 1, 1};
    Cell() {}
};

class Grid {
public:
    int width;
    int height;
    vector<vector<Cell>> grid;

    Grid(int widthLL, int heightLL) {
        width = widthLL;
        height = heightLL;
        grid.resize(height, vector<Cell>(width));
    }

    bool removeWall(int row, int column, short wall) {
        //clearing impossible cases
        if (row < 0 || column < 0 || row >= height || column >= width) return false;
        //checks if the required direction is a wall
        if (wall == 0 && row == 0) return false;
        if (wall == 1 && column == 0) return false;
        if (wall == 2 && column == width - 1) return false;
        if (wall == 3 && row == height - 1) return false;

        //eliminates the wall of the desired cell
        grid[row][column].walls[wall] = false;

        //disables the wall of the neighbor cell to be connected according to the wall short provided
        if (wall == 0) grid[row - 1][column].walls[3] = false;
        else if (wall == 1) grid[row][column - 1].walls[2] = false;
        else if (wall == 2) grid[row][column + 1].walls[1] = false;
        else if (wall == 3) grid[row + 1][column].walls[0] = false;

        return true;
    }
};


//SMFL graphic implementation
// Renders the provided grid in a blocking window loop (SFML 3-compatible).
void showMaze(Grid &grid, int cellSize, const std::string &title = "SFML Maze Grid") {
    unsigned int winW = static_cast<unsigned int>(grid.width * cellSize);
    unsigned int winH = static_cast<unsigned int>(grid.height * cellSize);

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(winW, winH)), title);

    auto drawLine = [&](float x1, float y1, float x2, float y2, const sf::Color &col) {
        // sf::Vertex is aggregate in SFML3 — set fields manually
        sf::Vertex verts[2];
        verts[0].position = sf::Vector2f(x1, y1);
        verts[0].color    = col;
        verts[1].position = sf::Vector2f(x2, y2);
        verts[1].color    = col;
        window.draw(verts, 2, sf::PrimitiveType::Lines);
    };

    while (window.isOpen()) {
        // pollEvent returns std::optional<sf::Event> in SFML3
        while (auto evOpt = window.pollEvent()) {
            const sf::Event &ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) window.close();
        }

        window.clear(sf::Color::Black);

        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                float xpos = x * cellSize;
                float ypos = y * cellSize;

                if (grid.grid[y][x].walls[0]) // Top
                    drawLine(xpos, ypos, xpos + cellSize, ypos, sf::Color::White);
                if (grid.grid[y][x].walls[1]) // Left
                    drawLine(xpos, ypos, xpos, ypos + cellSize, sf::Color::White);
                if (grid.grid[y][x].walls[2]) // Right
                    drawLine(xpos + cellSize, ypos, xpos + cellSize, ypos + cellSize, sf::Color::White);
                if (grid.grid[y][x].walls[3]) // Bottom
                    drawLine(xpos, ypos + cellSize, xpos + cellSize, ypos + cellSize, sf::Color::White);
            }
        }

        window.display();
    }
}

int main() {
    // create a grid: Grid(width, height)
    Grid grid(10, 10);

    // sample wall removals to demonstrate
    // grid.removeWall(0, 0, 2);
    // grid.removeWall(0, 1, 3);
    // grid.removeWall(1, 1, 1);

    //diagonal test case for debugging
    cout<<"pick a wall";
    short wall;
    cin>>wall;
    
    for(int i = 0; i<10;i++){
        grid.removeWall(i,i,wall);
    }
    // show the maze in a window, cell size 40px
    showMaze(grid, 40, "SFML 3 Maze");

    return 0;
}
