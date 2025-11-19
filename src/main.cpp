// src/main.cpp
#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "MazeAlgorithm.h"
#include "Menu.h"
#include "MazeSolver.h"
#include "AStarSolver.h"
#include "GreedySolver.h"
#include "UCSSolver.h"

// include algorithm implementations (your project uses this pattern)
#include "DFSAlgorithm.cpp"
#include "PrimsAlgorithm.cpp"
#include "HuntAndKillAlgorithm.cpp"
#include "KruskalsAlgorithm.cpp"

#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <cstdint>
#include <limits>
#include <algorithm>

void runAlgorithm(Grid &grid, MazeAlgorithm &algo, int cellSize, const std::string &title, const sf::Font *fontPtr = nullptr) {
    unsigned int winW = static_cast<unsigned int>(grid.width() * cellSize);
    unsigned int winH = static_cast<unsigned int>(grid.height() * cellSize);
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(winW, winH)), title);
    window.setFramerateLimit(60);

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time stepTime = sf::milliseconds(8);

    // solver state: choose after generation finishes
    bool solverChosen = false;
    enum SolverType { SOLVER_NONE = 0, SOLVER_DFS, SOLVER_ASTAR, SOLVER_GREEDY, SOLVER_UCS };
    SolverType solverType = SOLVER_NONE;

    std::unique_ptr<MazeSolver> dfsSolver;
    std::unique_ptr<AStarSolver> aStarSolver;
    std::unique_ptr<GreedySolver> greedySolver;
    std::unique_ptr<UCSSolver> ucsSolver;

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
        // SFML 3 event polling
        while (auto evOpt = window.pollEvent()) {
            const sf::Event &ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) { window.close(); break; }
        }

        // update (generation first, then chosen solver)
        accumulator += clock.restart();
        while (accumulator >= stepTime) {
            if (!algo.finished()) {
                algo.step();
            } else {
                if (!solverChosen) {
                    // solver selection menu
                    std::vector<std::string> solverOptions = {
                        "DFS (stack) - uses your MazeSolver",
                        "A* (Manhattan) - incremental A*",
                        "Greedy Best-First (heuristic)",
                        "Uniform Cost Search (UCS)"
                    };
                    Menu solverMenu(solverOptions, "Choose Solver Algorithm");
                    int solverChoice = solverMenu.run(window);
                    if (solverChoice < 0) { window.close(); return; }

                    if (solverChoice == 0) {
                        dfsSolver = std::make_unique<MazeSolver>(grid, start, goal);
                        solverType = SOLVER_DFS;
                    } else if (solverChoice == 1) {
                        aStarSolver = std::make_unique<AStarSolver>(grid, start, goal);
                        solverType = SOLVER_ASTAR;
                    } else if (solverChoice == 2) {
                        greedySolver = std::make_unique<GreedySolver>(grid, start, goal);
                        solverType = SOLVER_GREEDY;
                    } else if (solverChoice == 3) {
                        ucsSolver = std::make_unique<UCSSolver>(grid, start, goal);
                        solverType = SOLVER_UCS;
                    } else {
                        dfsSolver = std::make_unique<MazeSolver>(grid, start, goal);
                        solverType = SOLVER_DFS;
                    }

                    solverChosen = true;
                } else {
                    // step the currently selected solver incrementally
                    if (solverType == SOLVER_DFS) {
                        if (dfsSolver && !dfsSolver->finished()) dfsSolver->step();
                    } else if (solverType == SOLVER_ASTAR) {
                        if (aStarSolver && !aStarSolver->finished()) aStarSolver->step();
                    } else if (solverType == SOLVER_GREEDY) {
                        if (greedySolver && !greedySolver->finished()) greedySolver->step();
                    } else if (solverType == SOLVER_UCS) {
                        if (ucsSolver && !ucsSolver->finished()) ucsSolver->step();
                    }
                }
            }
            accumulator -= stepTime;
        }

        // RENDER
        window.clear(sf::Color::Black);

        // base grid: visited background + walls
        for (int y = 0; y < grid.height(); ++y) {
            for (int x = 0; x < grid.width(); ++x) {
                float xpos = x * cellSize;
                float ypos = y * cellSize;

                if (grid.at(x, y).visited) {
                    sf::RectangleShape rect(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
                    rect.setPosition(sf::Vector2f(xpos, ypos));
                    rect.setFillColor(sf::Color(46, 46, 46));
                    window.draw(rect);
                }

                if (grid.at(x, y).walls[0]) drawLine(window, xpos, ypos, xpos + cellSize, ypos, sf::Color(220,220,220));
                if (grid.at(x, y).walls[1]) drawLine(window, xpos, ypos, xpos, ypos + cellSize, sf::Color(220,220,220));
                if (grid.at(x, y).walls[2]) drawLine(window, xpos + cellSize, ypos, xpos + cellSize, ypos + cellSize, sf::Color(220,220,220));
                if (grid.at(x, y).walls[3]) drawLine(window, xpos, ypos + cellSize, xpos + cellSize, ypos + cellSize, sf::Color(220,220,220));
            }
        }

        // generator current highlight
        Coord genCur;
        if (!algo.finished() && algo.getCurrent(genCur)) {
            sf::RectangleShape curRect(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
            curRect.setPosition(sf::Vector2f(static_cast<float>(genCur.x * cellSize), static_cast<float>(genCur.y * cellSize)));
            curRect.setFillColor(sf::Color(0, 160, 0, 140));
            window.draw(curRect);
        }

        // Solver visualization (OPEN/CLOSED heatmap + current)
        if (solverChosen) {
            // Helper lambda to draw states for solvers that expose getStateGrid/getGScoreGrid
            auto drawStateAndHeat = [&](auto &solverPtr) {
                if (!solverPtr) return;
                const auto &states = solverPtr->getStateGrid();
                const auto &gGrid = solverPtr->getGScoreGrid();

                // find max finite g for normalization
                float maxG = 1.0f;
                for (int yy = 0; yy < grid.height(); ++yy)
                    for (int xx = 0; xx < grid.width(); ++xx) {
                        float v = gGrid[xx][yy];
                        if (v < std::numeric_limits<float>::infinity() && v > maxG) maxG = v;
                    }

                for (int y = 0; y < grid.height(); ++y) {
                    for (int x = 0; x < grid.width(); ++x) {
                        float xpos = x * cellSize;
                        float ypos = y * cellSize;

                        // CLOSED == 2
                        if (states[x][y] == 2) {
                            sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                            r.setPosition(sf::Vector2f(xpos, ypos));
                            r.setFillColor(sf::Color(120, 80, 160, 100));
                            window.draw(r);
                        }

                        // OPEN == 1 -> heatmap
                        if (states[x][y] == 1) {
                            float v = gGrid[x][y];
                            float t = 0.0f;
                            if (v < std::numeric_limits<float>::infinity()) t = std::min(1.0f, v / maxG);
                            uint8_t rcol = static_cast<uint8_t>(180 * t + 70 * (1.0f - t));
                            uint8_t gcol = static_cast<uint8_t>(140 * (1.0f - t) + 80 * t);
                            uint8_t bcol = static_cast<uint8_t>(200 * (1.0f - t) + 30 * t);
                            sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                            r.setPosition(sf::Vector2f(xpos, ypos));
                            r.setFillColor(sf::Color(rcol, gcol, bcol, 120));
                            window.draw(r);
                        }
                    }
                }

                // current node highlight
                Coord cur;
                if (solverPtr->getCurrent(cur)) {
                    sf::RectangleShape s(sf::Vector2f((float)cellSize, (float)cellSize));
                    s.setPosition(sf::Vector2f((float)cur.x * cellSize, (float)cur.y * cellSize));
                    s.setFillColor(sf::Color(255, 200, 0, 170));
                    s.setOutlineThickness(2.0f);
                    s.setOutlineColor(sf::Color::White);
                    window.draw(s);
                }
            };

            // Draw based on selected solver
            if (solverType == SOLVER_ASTAR) drawStateAndHeat(aStarSolver);
            else if (solverType == SOLVER_GREEDY) drawStateAndHeat(greedySolver);
            else if (solverType == SOLVER_UCS) drawStateAndHeat(ucsSolver);

            // final path drawing for any solver
            const std::vector<Coord> *pathPtr = nullptr;
            if (solverType == SOLVER_DFS && dfsSolver) pathPtr = &dfsSolver->getSolution();
            if (solverType == SOLVER_ASTAR && aStarSolver) pathPtr = &aStarSolver->getSolution();
            if (solverType == SOLVER_GREEDY && greedySolver) pathPtr = &greedySolver->getSolution();
            if (solverType == SOLVER_UCS && ucsSolver) pathPtr = &ucsSolver->getSolution();

            if (pathPtr && !pathPtr->empty()) {
                for (const Coord &c : *pathPtr) {
                    sf::RectangleShape rect(sf::Vector2f((float)cellSize, (float)cellSize));
                    rect.setPosition(sf::Vector2f((float)c.x * cellSize, (float)c.y * cellSize));
                    rect.setFillColor(sf::Color(20, 160, 255, 200));
                    window.draw(rect);
                }
                // connecting lines for continuity
                for (size_t i = 1; i < pathPtr->size(); ++i) {
                    const Coord &a = (*pathPtr)[i-1];
                    const Coord &b = (*pathPtr)[i];
                    float ax = a.x * cellSize + cellSize * 0.5f;
                    float ay = a.y * cellSize + cellSize * 0.5f;
                    float bx = b.x * cellSize + cellSize * 0.5f;
                    float by = b.y * cellSize + cellSize * 0.5f;
                    sf::Vertex line[2];
                    line[0].position = sf::Vector2f(ax, ay);
                    line[0].color = sf::Color(10, 80, 200, 230);
                    line[1].position = sf::Vector2f(bx, by);
                    line[1].color = sf::Color(10, 80, 200, 230);
                    window.draw(line, 2, sf::PrimitiveType::Lines);
                }
            }
        }

        // overlay & legend
        if (fontPtr) {
            sf::Text titleText(*fontPtr, title, 18);
            titleText.setPosition(sf::Vector2f(6.f, 6.f));
            titleText.setFillColor(sf::Color(235,235,235,230));
            window.draw(titleText);

            // legend
            float lx = 8.f, ly = 30.f, lh = 14.f;
            sf::RectangleShape legendClosed(sf::Vector2f(18, lh));
            legendClosed.setPosition(sf::Vector2f(lx, ly));
            legendClosed.setFillColor(sf::Color(120,80,160,120));
            window.draw(legendClosed);
            sf::Text closedText(*fontPtr, "closed", 12);
            closedText.setPosition(sf::Vector2f(lx + 22.f, ly - 2.f));
            closedText.setFillColor(sf::Color(200,200,200,200));
            window.draw(closedText);

            sf::RectangleShape legendOpen(sf::Vector2f(18, lh));
            legendOpen.setPosition(sf::Vector2f(lx, ly + 18.f));
            legendOpen.setFillColor(sf::Color(200,160,80,120));
            window.draw(legendOpen);
            sf::Text openText(*fontPtr, "open (heat)", 12);
            openText.setPosition(sf::Vector2f(lx + 22.f, ly + 16.f));
            openText.setFillColor(sf::Color(200,200,200,200));
            window.draw(openText);

            sf::RectangleShape legendPath(sf::Vector2f(18, lh));
            legendPath.setPosition(sf::Vector2f(lx, ly + 36.f));
            legendPath.setFillColor(sf::Color(20,160,255,200));
            window.draw(legendPath);
            sf::Text pathText(*fontPtr, "path", 12);
            pathText.setPosition(sf::Vector2f(lx + 22.f, ly + 34.f));
            pathText.setFillColor(sf::Color(200,200,200,200));
            window.draw(pathText);
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
