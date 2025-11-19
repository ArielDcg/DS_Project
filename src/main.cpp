#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "MazeAlgorithm.h"
#include "Menu.h"
#include "MazeSolver.h"
#include "AStarSolver.h"
#include "GreedySolver.h"
#include "UCSSolver.h"

// nuevos includes
#include "RankingTree.h"
#include "LeaderboardUI.h"

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

    bool solverChosen = false;
    enum SolverType { SOLVER_NONE = 0, SOLVER_DFS, SOLVER_ASTAR, SOLVER_GREEDY, SOLVER_UCS };
    SolverType solverType = SOLVER_NONE;

    std::unique_ptr<MazeSolver> dfsSolver;
    std::unique_ptr<AStarSolver> aStarSolver;
    std::unique_ptr<GreedySolver> greedySolver;
    std::unique_ptr<UCSSolver> ucsSolver;

    Coord start(0, 0);
    Coord goal(grid.width() - 1, grid.height() - 1);

    // ranking + leaderboard
    RankingTree ranking(RankingMode::ByEfficiency);
    LeaderboardUI leaderboard;

    while (window.isOpen()) {
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
                        "DFS (stack)",
                        "A* (Manhattan)",
                        "Greedy Best-First",
                        "Uniform Cost Search (UCS)"
                    };
                    Menu solverMenu(solverOptions, "Choose Solver Algorithm");
                    int solverChoice = solverMenu.run(window);
                    if (solverChoice < 0) { window.close(); return; }

                    if (solverChoice == 0) { dfsSolver = std::make_unique<MazeSolver>(grid, start, goal); solverType = SOLVER_DFS; }
                    else if (solverChoice == 1) { aStarSolver = std::make_unique<AStarSolver>(grid, start, goal); solverType = SOLVER_ASTAR; }
                    else if (solverChoice == 2) { greedySolver = std::make_unique<GreedySolver>(grid, start, goal); solverType = SOLVER_GREEDY; }
                    else if (solverChoice == 3) { ucsSolver = std::make_unique<UCSSolver>(grid, start, goal); solverType = SOLVER_UCS; }

                    solverChosen = true;
                } else {
                    if (solverType == SOLVER_DFS && dfsSolver && !dfsSolver->finished()) dfsSolver->step();
                    else if (solverType == SOLVER_ASTAR && aStarSolver && !aStarSolver->finished()) aStarSolver->step();
                    else if (solverType == SOLVER_GREEDY && greedySolver && !greedySolver->finished()) greedySolver->step();
                    else if (solverType == SOLVER_UCS && ucsSolver && !ucsSolver->finished()) ucsSolver->step();

                    // cuando termine, registrar métricas
                    if (solverType == SOLVER_DFS && dfsSolver && dfsSolver->finished()) {
                        ranking.recordRun("DFS", dfsSolver->getElapsedMs(), dfsSolver->getNodesExplored(), dfsSolver->getSolution().size());
                    }
                    if (solverType == SOLVER_ASTAR && aStarSolver && aStarSolver->finished()) {
                        ranking.recordRun("A*", aStarSolver->getElapsedMs(), aStarSolver->getNodesExplored(), aStarSolver->getSolution().size());
                    }
                    if (solverType == SOLVER_GREEDY && greedySolver && greedySolver->finished()) {
                        ranking.recordRun("Greedy", greedySolver->getElapsedMs(), greedySolver->getNodesExplored(), greedySolver->getSolution().size());
                    }
                    if (solverType == SOLVER_UCS && ucsSolver && ucsSolver->finished()) {
                        ranking.recordRun("UCS", ucsSolver->getElapsedMs(), ucsSolver->getNodesExplored(), ucsSolver->getSolution().size());
                    }
                }
            }
            accumulator -= stepTime;
        }

        window.clear(sf::Color::Black);

        // ... (tu renderizado actual del grid y solver)

        // leaderboard overlay
        leaderboard.draw(window, ranking, 4);

        window.display();
    }
}

int main() {
    const int GRID_W = 40, GRID_H = 28, CELL_SIZE = 20;
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
        case 0: algo.reset(new DFSAlgorithm(grid)); title = "DFS (Recursive Backtracker)"; break;
        case 1: algo.reset(new PrimsAlgorithm(grid)); title = "Prim's"; break;
        case 2: algo.reset(new HuntAndKillAlgorithm(grid)); title = "Hunt and Kill"; break;
        case 3: algo.reset(new KruskalsAlgorithm(grid)); title = "Kruskal"; break;
        default: algo.reset(new DFSAlgorithm(grid)); title = "Fallback: DFS"; break;
    }

    const sf::Font *fontPtr = menu.isFontLoaded() ? &menu.getFont() : nullptr;
    runAlgorithm(grid, *algo, CELL_SIZE, title, fontPtr);

    return 0;
}

