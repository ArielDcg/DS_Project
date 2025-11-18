// src/main.cpp
#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "MazeAlgorithm.h"
#include "Menu.h"
#include "MazeSolver.h"
#include "AStarSolver.h"   // ensure this exists

// include algorithm implementations
#include "DFSAlgorithm.cpp"
#include "PrimsAlgorithm.cpp"
#include "HuntAndKillAlgorithm.cpp"
#include "KruskalsAlgorithm.cpp"

#include <memory>
#include <string>
#include <vector>
#include <ctime>

void runAlgorithm(Grid &grid, MazeAlgorithm &algo, int cellSize, const std::string &title, const sf::Font *fontPtr = nullptr) {
    unsigned int winW = static_cast<unsigned int>(grid.width() * cellSize);
    unsigned int winH = static_cast<unsigned int>(grid.height() * cellSize);
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(winW, winH)), title);
    window.setFramerateLimit(60);

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time stepTime = sf::milliseconds(8);

    // solver state
    bool solverChosen = false;
    enum SolverType { SOLVER_NONE = 0, SOLVER_DFS, SOLVER_ASTAR };
    SolverType solverType = SOLVER_NONE;

    std::unique_ptr<MazeSolver> dfsSolver;
    std::unique_ptr<AStarSolver> aStarSolver;

    Coord start(0, 0);
    Coord goal(grid.width() - 1, grid.height() - 1);

    auto drawLine = [&](sf::RenderTarget &target, float x1, float y1, float x2, float y2, const sf::Color &col) {
        sf::Vertex verts[2];
        verts[0].position = sf::Vector2f(x1, y1);
        verts[0].color = col;
        verts[1].position = sf::Vector2f(x2, y2);
        verts[1].color = col;
        target.draw(verts, 2, sf::PrimitiveType::Lines);
    };

    while (window.isOpen()) {
        // SFML3: pollEvent returns std::optional<sf::Event>
        while (auto evOpt = window.pollEvent()) {
            const sf::Event &ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) { window.close(); break; }
        }

        accumulator += clock.restart();
        while (accumulator >= stepTime) {
            if (!algo.finished()) {
                algo.step();
            } else {
                if (!solverChosen) {
                    std::vector<std::string> solverOptions = {
                        "DFS (stack) - uses your MazeSolver",
                        "A* (Manhattan) - incremental A*"
                    };
                    Menu solverMenu(solverOptions, "Choose Solver Algorithm");
                    int solverChoice = solverMenu.run(window);
                    if (solverChoice < 0) {
                        window.close();
                        return;
                    }

                    if (solverChoice == 0) {
                        dfsSolver = std::make_unique<MazeSolver>(grid, start, goal);
                        solverType = SOLVER_DFS;
                    } else {
                        aStarSolver = std::make_unique<AStarSolver>(grid, start, goal);
                        solverType = SOLVER_ASTAR;
                    }
                    solverChosen = true;
                } else {
                    if (solverType == SOLVER_DFS) {
                        if (dfsSolver && !dfsSolver->finished()) dfsSolver->step();
                    } else if (solverType == SOLVER_ASTAR) {
                        if (aStarSolver && !aStarSolver->finished()) aStarSolver->step();
                    }
                }
            }
            accumulator -= stepTime;
        }

        window.clear(sf::Color::Black);

        for (int y = 0; y < grid.height(); ++y) {
            for (int x = 0; x < grid.width(); ++x) {
                float xpos = x * cellSize;
                float ypos = y * cellSize;

                if (grid.at(x, y).visited) {
                    sf::RectangleShape rect(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
                    rect.setPosition(sf::Vector2f(xpos, ypos));
                    rect.setFillColor(sf::Color(60, 60, 60));
                    window.draw(rect);
                }

                if (grid.at(x, y).walls[0]) drawLine(window, xpos, ypos, xpos + cellSize, ypos, sf::Color::White);
                if (grid.at(x, y).walls[1]) drawLine(window, xpos, ypos, xpos, ypos + cellSize, sf::Color::White);
                if (grid.at(x, y).walls[2]) drawLine(window, xpos + cellSize, ypos, xpos + cellSize, ypos + cellSize, sf::Color::White);
                if (grid.at(x, y).walls[3]) drawLine(window, xpos, ypos + cellSize, xpos + cellSize, ypos + cellSize, sf::Color::White);
            }
        }

        // generator current
        Coord genCur;
        if (!algo.finished() && algo.getCurrent(genCur)) {
            sf::RectangleShape curRect(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
            curRect.setPosition(sf::Vector2f(static_cast<float>(genCur.x * cellSize), static_cast<float>(genCur.y * cellSize)));
            curRect.setFillColor(sf::Color(0, 160, 0, 160));
            window.draw(curRect);
        }

        // solver visuals
        if (solverChosen) {
            Coord cur;
            if (solverType == SOLVER_DFS && dfsSolver && dfsSolver->getCurrent(cur)) {
                sf::RectangleShape curRect(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
                curRect.setPosition(sf::Vector2f(static_cast<float>(cur.x * cellSize), static_cast<float>(cur.y * cellSize)));
                curRect.setFillColor(sf::Color(200, 120, 0, 160));
                window.draw(curRect);
            } else if (solverType == SOLVER_ASTAR && aStarSolver && aStarSolver->getCurrent(cur)) {
                sf::RectangleShape curRect(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
                curRect.setPosition(sf::Vector2f(static_cast<float>(cur.x * cellSize), static_cast<float>(cur.y * cellSize)));
                curRect.setFillColor(sf::Color(200, 120, 0, 160));
                window.draw(curRect);
            }

            // final path
            if ((solverType == SOLVER_DFS && dfsSolver && dfsSolver->finished()) ||
                (solverType == SOLVER_ASTAR && aStarSolver && aStarSolver->finished())) {

                const std::vector<Coord> *pathPtr = nullptr;
                if (solverType == SOLVER_DFS && dfsSolver) pathPtr = &dfsSolver->getSolution();
                if (solverType == SOLVER_ASTAR && aStarSolver) pathPtr = &aStarSolver->getSolution();

                if (pathPtr) {
                    for (const Coord &c : *pathPtr) {
                        sf::RectangleShape rect(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
                        rect.setPosition(sf::Vector2f(static_cast<float>(c.x * cellSize), static_cast<float>(c.y * cellSize)));
                        rect.setFillColor(sf::Color(0, 120, 255, 160));
                        window.draw(rect);
                    }
                }
            }
        }

        // draw title if font provided
        if (fontPtr) {
            sf::Text t(*fontPtr, title, 16);
            t.setPosition(sf::Vector2f(4.f, 4.f));
            t.setFillColor(sf::Color(200, 200, 200));
            window.draw(t);
        }

        window.display();
    }
}

int main() {
    const int GRID_W = 40;
    const int GRID_H = 28;
    const int CELL_SIZE = 20;

    sf::RenderWindow menuWindow(sf::VideoMode(sf::Vector2u(800, 600)), "Maze - Select Algorithm");
    menuWindow.setFramerateLimit(60);

    Menu menu;
    int choice = menu.run(menuWindow);
    if (choice < 0) return 0;

    Grid grid(GRID_W, GRID_H);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::unique_ptr<MazeAlgorithm> algo;
    std::string title;

    switch (choice) {
        case 0:
            algo.reset(new DFSAlgorithm(grid));
            title = "DFS (Recursive Backtracker)";
            break;
        case 1:
            algo.reset(new PrimsAlgorithm(grid));
            title = "Prim's (incremental)";
            break;
        case 2:
            algo.reset(new HuntAndKillAlgorithm(grid));
            title = "Hunt and Kill Algorithm";
            break;
        case 3:
            algo.reset(new KruskalsAlgorithm(grid));
            title = "KruskalsAlgorithm";
            break;
        default:
            algo.reset(new DFSAlgorithm(grid));
            title = "Fallback: DFS";
            break;
    }

    const sf::Font *fontPtr = menu.isFontLoaded() ? &menu.getFont() : nullptr;
    runAlgorithm(grid, *algo, CELL_SIZE, title, fontPtr);

    return 0;
}
