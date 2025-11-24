// main.cpp - MODO COLECCIONISTA COMPLETO CON LOOP CONTINUO
#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "MazeAlgorithm.h"
#include "Menu.h"
#include "MazeSolver.h"
#include "AStarSolver.h"
#include "GreedySolver.h"
#include "UCSSolver.h"
#include "ChallengeSystem.h"
#include "CollectorSolver.h"
#include "AlgorithmRankingAVL.h"
#include "ExplorationHeatmap.h"

#include "DFSAlgorithm.cpp"
#include "PrimsAlgorithm.cpp"
#include "HuntAndKillAlgorithm.cpp"
#include "KruskalsAlgorithm.cpp"
#include "CollectorMazeGen.cpp"

#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <chrono>
#include <iostream>

// Forward declarations
void displayHeatmap(const Grid& grid, const ExplorationHeatmap& heatmap,
                   const sf::Font* fontPtr, const std::string& windowTitle);

std::string getStrategyName(SolverStrategy strategy) {
    switch (strategy) {
        case SolverStrategy::ASTAR: return "A*";
        case SolverStrategy::GREEDY: return "Greedy";
        case SolverStrategy::UCS: return "UCS";
        case SolverStrategy::DFS: return "DFS";
        default: return "Unknown";
    }
}

struct ObjectiveColors {
    sf::Color closed;
    sf::Color path;
};

ObjectiveColors getColorsForObjective(int objectiveIndex) {
    ObjectiveColors colors;
    switch (objectiveIndex) {
        case 0:
            colors.closed = sf::Color(120, 80, 160, 120);
            colors.path = sf::Color(20, 160, 255, 200);
            break;
        case 1:
            colors.closed = sf::Color(217, 121, 64, 120);
            colors.path = sf::Color(64, 217, 96, 200);
            break;
        case 2:
            colors.closed = sf::Color(64, 192, 217, 120);
            colors.path = sf::Color(255, 215, 0, 200);
            break;
        case 3:
            colors.closed = sf::Color(217, 64, 217, 120);
            colors.path = sf::Color(255, 64, 64, 200);
            break;
        default:
            colors.closed = sf::Color(120, 80, 160, 120);
            colors.path = sf::Color(20, 160, 255, 200);
            break;
    }
    return colors;
}

void runCollectorMode(Grid &grid, MazeAlgorithm &algo, ChallengeSystem &challenges,
                      Coord start, Coord goal, int cellSize, const std::string &title,
                      SolverStrategy strategy, const sf::Font *fontPtr = nullptr) {
    unsigned int winW = static_cast<unsigned int>(grid.width() * cellSize);
    unsigned int winH = static_cast<unsigned int>(grid.height() * cellSize);
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(winW, winH)), 
                           title + " - COLLECTOR [" + getStrategyName(strategy) + "]");
    window.setFramerateLimit(60);

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time stepTime = sf::milliseconds(8);

    std::unique_ptr<CollectorSolver> solver;
    bool solverInitialized = false;

    auto drawLine = [&](sf::RenderTarget &target, float x1, float y1, float x2, float y2, const sf::Color &col) {
        sf::Vertex verts[2];
        verts[0].position = sf::Vector2f(x1, y1);
        verts[0].color = col;
        verts[1].position = sf::Vector2f(x2, y2);
        verts[1].color = col;
        target.draw(verts, 2, sf::PrimitiveType::Lines);
    };

    while (window.isOpen()) {
        while (auto evOpt = window.pollEvent()) {
            const sf::Event &ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) {
                window.close();
                return;  // Volver al menú
            }

            // Permitir ver heatmap cuando el solver ha terminado
            if (ev.is<sf::Event::KeyPressed>()) {
                const auto* keyEv = ev.getIf<sf::Event::KeyPressed>();
                if (solverInitialized && solver && solver->finished() &&
                    keyEv && keyEv->code == sf::Keyboard::Key::H) {
                    displayHeatmap(grid, solver->getHeatmap(), fontPtr,
                                  title + " - Exploration Heatmap [" + getStrategyName(strategy) + "]");
                }
            }
        }

        accumulator += clock.restart();
        while (accumulator >= stepTime) {
            if (!algo.finished()) {
                algo.step();
            } else if (!solverInitialized) {
                solver = std::make_unique<CollectorSolver>(grid, challenges, start, goal, strategy);
                solverInitialized = true;
            } else if (!solver->finished()) {
                solver->step();
            }
            accumulator -= stepTime;
        }

        window.clear(sf::Color::Black);

        // Grid
        for (int y = 0; y < grid.height(); ++y) {
            for (int x = 0; x < grid.width(); ++x) {
                float xpos = x * cellSize;
                float ypos = y * cellSize;

                if (grid.at(x, y).visited) {
                    sf::RectangleShape rect(sf::Vector2f((float)cellSize, (float)cellSize));
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

        // Tesoros
        const std::vector<Coord>& treasures = challenges.getTreasurePositions();
        for (const Coord& t : treasures) {
            if (challenges.hasTreasure(t)) {
                sf::CircleShape circle(cellSize * 0.35f);
                circle.setPosition(sf::Vector2f(t.x * cellSize + cellSize * 0.15f, t.y * cellSize + cellSize * 0.15f));
                circle.setFillColor(sf::Color(255, 215, 0, 220));
                circle.setOutlineThickness(2.0f);
                circle.setOutlineColor(sf::Color(255, 165, 0));
                window.draw(circle);
                
                sf::RectangleShape innerSquare(sf::Vector2f(cellSize * 0.2f, cellSize * 0.2f));
                innerSquare.setPosition(sf::Vector2f(t.x * cellSize + cellSize * 0.4f, t.y * cellSize + cellSize * 0.4f));
                innerSquare.setFillColor(sf::Color(255, 255, 0, 255));
                window.draw(innerSquare);
            } else {
                sf::RectangleShape marker(sf::Vector2f((float)cellSize, (float)cellSize));
                marker.setPosition(sf::Vector2f((float)t.x * cellSize, (float)t.y * cellSize));
                marker.setFillColor(sf::Color(100, 255, 100, 80));
                window.draw(marker);
            }
        }

        // Inicio y Meta
        sf::CircleShape startMarker(cellSize * 0.4f);
        startMarker.setPosition(sf::Vector2f(start.x * cellSize + cellSize * 0.1f, start.y * cellSize + cellSize * 0.1f));
        startMarker.setFillColor(sf::Color(0, 255, 0, 150));
        window.draw(startMarker);

        sf::CircleShape goalMarker(cellSize * 0.4f);
        goalMarker.setPosition(sf::Vector2f(goal.x * cellSize + cellSize * 0.1f, goal.y * cellSize + cellSize * 0.1f));
        goalMarker.setFillColor(sf::Color(255, 0, 0, 150));
        window.draw(goalMarker);

        // Generación
        Coord genCur;
        if (!algo.finished() && algo.getCurrent(genCur)) {
            sf::RectangleShape curRect(sf::Vector2f((float)cellSize, (float)cellSize));
            curRect.setPosition(sf::Vector2f((float)genCur.x * cellSize, (float)genCur.y * cellSize));
            curRect.setFillColor(sf::Color(0, 160, 0, 140));
            window.draw(curRect);
        }

        // Solver
        if (solverInitialized && solver) {
            const auto& states = solver->getStateGrid();
            const auto& gGrid = solver->getGScoreGrid();
            int objIdx = solver->getCurrentObjectiveIndex();
            ObjectiveColors colors = getColorsForObjective(objIdx);

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

                    if (states[x][y] == CollectorSolver::CLOSED) {
                        sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                        r.setPosition(sf::Vector2f(xpos, ypos));
                        r.setFillColor(colors.closed);
                        window.draw(r);
                    }

                    if (states[x][y] == CollectorSolver::OPEN) {
                        float v = gGrid[x][y];
                        float t = 0.0f;
                        if (v < std::numeric_limits<float>::infinity()) t = std::min(1.0f, v / maxG);
                        
                        sf::Color baseColor = colors.closed;
                        uint8_t rcol = static_cast<uint8_t>(baseColor.r * t + 120 * (1.0f - t));
                        uint8_t gcol = static_cast<uint8_t>(baseColor.g * t + 100 * (1.0f - t));
                        uint8_t bcol = static_cast<uint8_t>(baseColor.b * t + 80 * (1.0f - t));
                        
                        sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                        r.setPosition(sf::Vector2f(xpos, ypos));
                        r.setFillColor(sf::Color(rcol, gcol, bcol, 120));
                        window.draw(r);
                    }
                }
            }

            Coord cur;
            if (solver->getCurrent(cur)) {
                sf::RectangleShape s(sf::Vector2f((float)cellSize, (float)cellSize));
                s.setPosition(sf::Vector2f((float)cur.x * cellSize, (float)cur.y * cellSize));
                s.setFillColor(sf::Color(255, 200, 0, 170));
                s.setOutlineThickness(2.0f);
                s.setOutlineColor(sf::Color::White);
                window.draw(s);
            }

            // SEGMENTOS CON COLORES PERSISTENTES
            const std::vector<PathSegment>& segments = solver->getSegments();
            for (const PathSegment& seg : segments) {
                ObjectiveColors segColors = getColorsForObjective(seg.objectiveIndex);
                
                for (const Coord& c : seg.path) {
                    sf::RectangleShape rect(sf::Vector2f((float)cellSize, (float)cellSize));
                    rect.setPosition(sf::Vector2f((float)c.x * cellSize, (float)c.y * cellSize));
                    rect.setFillColor(segColors.path);
                    window.draw(rect);
                }
                
                // Tesoro bonus (verde brillante)
                if (seg.foundBonus) {
                    sf::CircleShape bonus(cellSize * 0.4f);
                    bonus.setPosition(sf::Vector2f(seg.bonusTreasure.x * cellSize + cellSize * 0.1f, 
                                                    seg.bonusTreasure.y * cellSize + cellSize * 0.1f));
                    bonus.setFillColor(sf::Color(0, 255, 0, 200));
                    bonus.setOutlineThickness(3.0f);
                    bonus.setOutlineColor(sf::Color(255, 255, 255));
                    window.draw(bonus);
                }
                
                for (size_t i = 1; i < seg.path.size(); ++i) {
                    const Coord& a = seg.path[i-1];
                    const Coord& b = seg.path[i];
                    float ax = a.x * cellSize + cellSize * 0.5f;
                    float ay = a.y * cellSize + cellSize * 0.5f;
                    float bx = b.x * cellSize + cellSize * 0.5f;
                    float by = b.y * cellSize + cellSize * 0.5f;
                    drawLine(window, ax, ay, bx, by, sf::Color(10, 80, 200, 200));
                }
            }
        }

        // Overlay
        if (fontPtr) {
            sf::Text titleText(*fontPtr, title + " - " + getStrategyName(strategy), 16);
            titleText.setPosition(sf::Vector2f(6.f, 6.f));
            titleText.setFillColor(sf::Color(235,235,235,230));
            window.draw(titleText);

            if (solverInitialized) {
                std::string treasureText = "Treasures: " + std::to_string(solver->getTreasuresCollected()) + "/3";
                sf::Text tText(*fontPtr, treasureText, 14);
                tText.setPosition(sf::Vector2f(6.f, 26.f));
                tText.setFillColor(sf::Color(255, 215, 0));
                window.draw(tText);
                
                int objIdx = solver->getCurrentObjectiveIndex();
                std::string objText = "Objective: ";
                if (objIdx < 3) objText += "Treasure " + std::to_string(objIdx + 1);
                else objText += "GOAL";
                sf::Text oText(*fontPtr, objText, 12);
                oText.setPosition(sf::Vector2f(6.f, 44.f));
                oText.setFillColor(sf::Color(200, 200, 200));
                window.draw(oText);

                // Mostrar hint para ver heatmap cuando termine
                if (solver->finished()) {
                    sf::Text hintText(*fontPtr, "Press H to view Exploration Heatmap", 12);
                    hintText.setPosition(sf::Vector2f(6.f, 64.f));
                    hintText.setFillColor(sf::Color(100, 200, 255));
                    window.draw(hintText);
                }
            }
        }

        window.display();
    }
}

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

    auto drawLine = [&](sf::RenderTarget &target, float x1, float y1, float x2, float y2, const sf::Color &col) {
        sf::Vertex verts[2];
        verts[0].position = sf::Vector2f(x1, y1);
        verts[0].color = col;
        verts[1].position = sf::Vector2f(x2, y2);
        verts[1].color = col;
        target.draw(verts, 2, sf::PrimitiveType::Lines);
    };

    while (window.isOpen()) {
        while (auto evOpt = window.pollEvent()) {
            const sf::Event &ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) { 
                window.close(); 
                return;  // Volver al menú
            }
        }

        accumulator += clock.restart();
        while (accumulator >= stepTime) {
            if (!algo.finished()) {
                algo.step();
            } else {
                if (!solverChosen) {
                    std::vector<std::string> solverOptions = {
                        "DFS",
                        "A*",
                        "Greedy Best-First",
                        "Uniform Cost Search"
                    };
                    Menu solverMenu(solverOptions, "Choose Solver");
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
                    }

                    solverChosen = true;
                } else {
                    if (solverType == SOLVER_DFS && dfsSolver && !dfsSolver->finished()) dfsSolver->step();
                    else if (solverType == SOLVER_ASTAR && aStarSolver && !aStarSolver->finished()) aStarSolver->step();
                    else if (solverType == SOLVER_GREEDY && greedySolver && !greedySolver->finished()) greedySolver->step();
                    else if (solverType == SOLVER_UCS && ucsSolver && !ucsSolver->finished()) ucsSolver->step();
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
                    rect.setFillColor(sf::Color(46, 46, 46));
                    window.draw(rect);
                }

                if (grid.at(x, y).walls[0]) drawLine(window, xpos, ypos, xpos + cellSize, ypos, sf::Color(220,220,220));
                if (grid.at(x, y).walls[1]) drawLine(window, xpos, ypos, xpos, ypos + cellSize, sf::Color(220,220,220));
                if (grid.at(x, y).walls[2]) drawLine(window, xpos + cellSize, ypos, xpos + cellSize, ypos + cellSize, sf::Color(220,220,220));
                if (grid.at(x, y).walls[3]) drawLine(window, xpos, ypos + cellSize, xpos + cellSize, ypos + cellSize, sf::Color(220,220,220));
            }
        }

        Coord genCur;
        if (!algo.finished() && algo.getCurrent(genCur)) {
            sf::RectangleShape curRect(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
            curRect.setPosition(sf::Vector2f(static_cast<float>(genCur.x * cellSize), static_cast<float>(genCur.y * cellSize)));
            curRect.setFillColor(sf::Color(0, 160, 0, 140));
            window.draw(curRect);
        }

        if (solverChosen) {
            auto drawStateAndHeat = [&](auto &solverPtr) {
                if (!solverPtr) return;
                const auto &states = solverPtr->getStateGrid();
                const auto &gGrid = solverPtr->getGScoreGrid();

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

                        if (states[x][y] == 2) {
                            sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                            r.setPosition(sf::Vector2f(xpos, ypos));
                            r.setFillColor(sf::Color(120, 80, 160, 100));
                            window.draw(r);
                        }

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

            if (solverType == SOLVER_ASTAR) drawStateAndHeat(aStarSolver);
            else if (solverType == SOLVER_GREEDY) drawStateAndHeat(greedySolver);
            else if (solverType == SOLVER_UCS) drawStateAndHeat(ucsSolver);

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

        if (fontPtr) {
            sf::Text titleText(*fontPtr, title, 18);
            titleText.setPosition(sf::Vector2f(6.f, 6.f));
            titleText.setFillColor(sf::Color(235,235,235,230));
            window.draw(titleText);
        }

        window.display();
    }
}

// ===================================
// BENCHMARK Y RANKING DE ALGORITMOS
// ===================================

// Ejecutar UN benchmark: un solver en un laberinto generado
AlgorithmStats runSingleBenchmark(const std::string& genAlgo, SolverStrategy strategy,
                                  const std::string& solverName, int gridW, int gridH) {
    AlgorithmStats stats;
    stats.algorithmName = solverName;

    Grid grid(gridW, gridH);
    ChallengeSystem challenges(grid);

    Coord start(gridW / 2, gridH / 2);
    Coord goal = getRandomCorner(grid, start);

    // Generar laberinto (sin visualización, sin contar tiempo)
    std::unique_ptr<MazeAlgorithm> algo;
    if (genAlgo == "DFS") {
        algo.reset(new DFSCollectorAlgorithm(grid, &challenges));
    } else if (genAlgo == "Prim's") {
        algo.reset(new PrimsCollectorAlgorithm(grid, &challenges));
    } else if (genAlgo == "Hunt&Kill") {
        algo.reset(new HuntAndKillCollectorAlgorithm(grid, &challenges));
    } else if (genAlgo == "Kruskal's") {
        algo.reset(new KruskalsCollectorAlgorithm(grid, &challenges));
    }

    while (!algo->finished()) {
        algo->step();
    }

    // Resolver laberinto (SOLO medir tiempo del solving)
    auto startTime = std::chrono::high_resolution_clock::now();
    CollectorSolver solver(grid, challenges, start, goal, strategy);

    while (!solver.finished()) {
        solver.step();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = endTime - startTime;

    // Recopilar estadísticas REALES
    stats.nodesExpanded = solver.getNodesExpanded();  // Contador real
    stats.pathLength = static_cast<int>(solver.getFullPath().size());
    stats.executionTime = duration.count();
    stats.treasuresCollected = solver.getTreasuresCollected();

    return stats;
}

void runAlgorithmRanking(sf::RenderWindow& window, const sf::Font* fontPtr) {
    const int GRID_W = 40;
    const int GRID_H = 28;
    const int ITERATIONS_PER_MAZE_TYPE = 30;  // Iteraciones por cada tipo de laberinto

    std::vector<std::string> genAlgos = {"DFS", "Prim's", "Hunt&Kill", "Kruskal's"};
    std::vector<std::pair<SolverStrategy, std::string>> solvers = {
        {SolverStrategy::ASTAR, "A*"},
        {SolverStrategy::GREEDY, "Greedy"},
        {SolverStrategy::UCS, "UCS"},
        {SolverStrategy::DFS, "DFS"}
    };

    AlgorithmRankingAVL ranking;

    std::cout << "\n=== Ejecutando Benchmark de Solvers ===\n";
    std::cout << "Cada solver se prueba en laberintos generados por " << genAlgos.size() << " algoritmos diferentes\n";
    std::cout << "Iteraciones por tipo de laberinto: " << ITERATIONS_PER_MAZE_TYPE << "\n";
    std::cout << "Total de tests por solver: " << genAlgos.size() * ITERATIONS_PER_MAZE_TYPE << "\n\n";

    // Ejecutar benchmarks SOLO para solvers
    for (const auto& solverPair : solvers) {
        std::cout << "Testing solver: " << solverPair.second << "...\n";

        AlgorithmStats avgStats;
        avgStats.algorithmName = solverPair.second;

        int totalTests = 0;

        // Probar en todos los tipos de laberintos
        for (const auto& genAlgo : genAlgos) {
            std::cout << "  " << genAlgo << " laberintos... ";

            for (int i = 0; i < ITERATIONS_PER_MAZE_TYPE; i++) {
                AlgorithmStats stats = runSingleBenchmark(genAlgo, solverPair.first, solverPair.second, GRID_W, GRID_H);
                avgStats.nodesExpanded += stats.nodesExpanded;
                avgStats.pathLength += stats.pathLength;
                avgStats.executionTime += stats.executionTime;
                avgStats.treasuresCollected += stats.treasuresCollected;

                totalTests++;
            }

            std::cout << "OK\n";
        }

        // Calcular promedios
        avgStats.nodesExpanded /= totalTests;
        avgStats.pathLength /= totalTests;
        avgStats.executionTime /= totalTests;
        avgStats.treasuresCollected /= totalTests;

        ranking.insert(avgStats);
        std::cout << "  Score final: " << avgStats.calculateScore() << "\n\n";
    }

    std::cout << "=== Benchmark Completado ===\n\n";

    // Mostrar resultados en ventana
    std::vector<RankEntry> topRankings = ranking.getAll();

    sf::RenderWindow rankWindow(sf::VideoMode(sf::Vector2u(1000, 700)), "Algorithm Ranking Results");
    rankWindow.setFramerateLimit(60);

    while (rankWindow.isOpen()) {
        while (auto evOpt = rankWindow.pollEvent()) {
            const sf::Event& ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) {
                rankWindow.close();
                return;
            }
        }

        rankWindow.clear(sf::Color(30, 30, 40));

        if (fontPtr) {
            // Título
            sf::Text titleText(*fontPtr, "Algorithm Ranking (AVL Tree)", 28);
            titleText.setPosition(sf::Vector2f(30, 20));
            titleText.setFillColor(sf::Color(255, 215, 0));
            rankWindow.draw(titleText);

            // Subtítulo
            int totalTests = 4 * ITERATIONS_PER_MAZE_TYPE;
            sf::Text subtitle(*fontPtr, "Cada solver probado en " + std::to_string(totalTests) + " laberintos (4 tipos x " + std::to_string(ITERATIONS_PER_MAZE_TYPE) + " iteraciones)", 14);
            subtitle.setPosition(sf::Vector2f(30, 55));
            subtitle.setFillColor(sf::Color(180, 180, 180));
            rankWindow.draw(subtitle);

            // Encabezados
            float yPos = 100;
            sf::Text header(*fontPtr, "Rank  Solver Algorithm           Score    Nodes   Path   Time(ms)  Treasures", 14);
            header.setPosition(sf::Vector2f(30, yPos));
            header.setFillColor(sf::Color(200, 200, 200));
            header.setStyle(sf::Text::Bold);
            rankWindow.draw(header);

            yPos += 40;

            // Resultados
            for (size_t i = 0; i < topRankings.size(); i++) {
                const RankEntry& entry = topRankings[i];

                // Color según ranking (mejor = verde, peor = rojo)
                sf::Color rowColor;
                if (i == 0) rowColor = sf::Color(100, 255, 100);      // 1º - Verde brillante
                else if (i == 1) rowColor = sf::Color(150, 220, 100); // 2º - Verde amarillento
                else if (i == 2) rowColor = sf::Color(255, 200, 100); // 3º - Naranja
                else rowColor = sf::Color(255, 150, 150);             // 4º - Rojo

                std::string line = std::to_string(entry.rank) + ".    " + entry.stats.algorithmName;

                // Padding para alinear
                while (line.length() < 33) line += " ";

                line += std::to_string(static_cast<int>(entry.stats.calculateScore())) + "     ";
                line += std::to_string(entry.stats.nodesExpanded) + "      ";
                line += std::to_string(entry.stats.pathLength) + "     ";
                line += std::to_string(static_cast<int>(entry.stats.executionTime * 1000)) + "       ";
                line += std::to_string(entry.stats.treasuresCollected);

                sf::Text rankText(*fontPtr, line, 16);
                rankText.setPosition(sf::Vector2f(30, yPos));
                rankText.setFillColor(rowColor);
                rankWindow.draw(rankText);

                yPos += 35;
            }

            // Explicación
            yPos += 20;
            std::vector<std::string> explanations = {
                "Score = 1000 - (nodos*0.5) - (path*2) - (tiempo*500) + (tesoros*50)",
                "Nodes = Nodos expandidos (celdas marcadas como CLOSED)",
                "Path = Longitud del camino final completo",
                "Time = Tiempo promedio de resoluci\u00f3n en milisegundos"
            };

            for (const auto& expl : explanations) {
                sf::Text explText(*fontPtr, expl, 11);
                explText.setPosition(sf::Vector2f(30, yPos));
                explText.setFillColor(sf::Color(150, 150, 150));
                rankWindow.draw(explText);
                yPos += 18;
            }

            // Instrucciones
            sf::Text hint(*fontPtr, "Close window to return to menu", 14);
            hint.setPosition(sf::Vector2f(30, 650));
            hint.setFillColor(sf::Color(150, 150, 150));
            rankWindow.draw(hint);

            // Info del árbol AVL
            sf::Text treeInfo(*fontPtr, "AVL Tree - Solvers: " + std::to_string(ranking.size()) +
                             " | Height: " + std::to_string(ranking.getHeight()) +
                             " | Balanced: " + (ranking.isBalanced() ? "Yes" : "No"), 12);
            treeInfo.setPosition(sf::Vector2f(30, 625));
            treeInfo.setFillColor(sf::Color(255, 215, 0));
            rankWindow.draw(treeInfo);
        }

        rankWindow.display();
    }
}

void showHeatmapVisualization(sf::RenderWindow& window, const sf::Font* fontPtr) {
    const int GRID_W = 40;
    const int GRID_H = 28;
    const int CELL_SIZE = 20;

    // Generar laberinto y resolver con A*
    Grid grid(GRID_W, GRID_H);
    ChallengeSystem challenges(grid);

    Coord start(GRID_W / 2, GRID_H / 2);
    Coord goal = getRandomCorner(grid, start);

    std::unique_ptr<MazeAlgorithm> algo(new DFSCollectorAlgorithm(grid, &challenges));
    while (!algo->finished()) {
        algo->step();
    }

    CollectorSolver solver(grid, challenges, start, goal, SolverStrategy::ASTAR);
    while (!solver.finished()) {
        solver.step();
    }

    const ExplorationHeatmap& heatmap = solver.getHeatmap();

    // Mostrar usando la función auxiliar
    displayHeatmap(grid, heatmap, fontPtr, "Exploration Heatmap (Sparse Matrix)");
}

// Función auxiliar para mostrar un heatmap dado
void displayHeatmap(const Grid& grid, const ExplorationHeatmap& heatmap,
                   const sf::Font* fontPtr, const std::string& windowTitle) {
    const int CELL_SIZE = 20;
    const int GRID_W = grid.width();
    const int GRID_H = grid.height();

    ExplorationHeatmap::HeatmapStats stats = heatmap.getStats();
    std::vector<std::vector<float>> normalizedGrid = heatmap.toNormalizedGrid();

    // Ventana de visualización
    sf::RenderWindow heatWindow(sf::VideoMode(sf::Vector2u(GRID_W * CELL_SIZE + 300, GRID_H * CELL_SIZE)),
                                windowTitle);
    heatWindow.setFramerateLimit(60);

    while (heatWindow.isOpen()) {
        while (auto evOpt = heatWindow.pollEvent()) {
            const sf::Event& ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) {
                heatWindow.close();
                return;
            }
        }

        heatWindow.clear(sf::Color(20, 20, 20));

        // Dibujar heatmap
        for (int y = 0; y < GRID_H; y++) {
            for (int x = 0; x < GRID_W; x++) {
                float intensity = normalizedGrid[x][y];

                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(sf::Vector2f(x * CELL_SIZE, y * CELL_SIZE));

                // Gradiente: azul (frío) -> verde -> amarillo -> rojo (caliente)
                sf::Color cellColor;
                if (intensity == 0.0f) {
                    cellColor = sf::Color(10, 10, 10);  // No visitado
                } else if (intensity < 0.33f) {
                    float t = intensity / 0.33f;
                    cellColor = sf::Color(0, static_cast<int>(t * 100), static_cast<int>(100 + t * 155));
                } else if (intensity < 0.66f) {
                    float t = (intensity - 0.33f) / 0.33f;
                    cellColor = sf::Color(static_cast<int>(t * 255), static_cast<int>(100 + t * 155), static_cast<int>(255 - t * 255));
                } else {
                    float t = (intensity - 0.66f) / 0.34f;
                    cellColor = sf::Color(255, static_cast<int>(255 - t * 100), 0);
                }

                cell.setFillColor(cellColor);
                heatWindow.draw(cell);
            }
        }

        // Panel de información
        if (fontPtr) {
            float panelX = GRID_W * CELL_SIZE + 20;
            float yPos = 20;

            sf::Text title(*fontPtr, "Heatmap Stats", 20);
            title.setPosition(sf::Vector2f(panelX, yPos));
            title.setFillColor(sf::Color(255, 215, 0));
            heatWindow.draw(title);

            yPos += 40;

            std::vector<std::string> infoLines = {
                "Total Visits: " + std::to_string(stats.totalVisits),
                "Unique Cells: " + std::to_string(stats.uniqueCells),
                "Max Visits: " + std::to_string(stats.maxVisits),
                "Avg Visits: " + std::to_string(static_cast<int>(stats.avgVisits)),
                "Coverage: " + std::to_string(static_cast<int>(stats.densityPercent)) + "%",
                "",
                "Sparse Matrix:",
                "Size: " + std::to_string(stats.uniqueCells) + " entries",
                "vs Full: " + std::to_string(GRID_W * GRID_H),
                "Saved: " + std::to_string(GRID_W * GRID_H - stats.uniqueCells)
            };

            for (const auto& line : infoLines) {
                sf::Text text(*fontPtr, line, 14);
                text.setPosition(sf::Vector2f(panelX, yPos));
                text.setFillColor(sf::Color::White);
                heatWindow.draw(text);
                yPos += 25;
            }

            // Leyenda de colores
            yPos += 20;
            sf::Text legendTitle(*fontPtr, "Color Legend:", 16);
            legendTitle.setPosition(sf::Vector2f(panelX, yPos));
            legendTitle.setFillColor(sf::Color(255, 215, 0));
            heatWindow.draw(legendTitle);
            yPos += 30;

            std::vector<std::pair<std::string, sf::Color>> legend = {
                {"Low", sf::Color(0, 100, 255)},
                {"Medium", sf::Color(0, 255, 100)},
                {"High", sf::Color(255, 255, 0)},
                {"Very High", sf::Color(255, 100, 0)}
            };

            for (const auto& item : legend) {
                sf::RectangleShape colorBox(sf::Vector2f(20, 20));
                colorBox.setPosition(sf::Vector2f(panelX, yPos));
                colorBox.setFillColor(item.second);
                heatWindow.draw(colorBox);

                sf::Text label(*fontPtr, item.first, 12);
                label.setPosition(sf::Vector2f(panelX + 30, yPos + 3));
                label.setFillColor(sf::Color::White);
                heatWindow.draw(label);
                yPos += 30;
            }

            // Hint
            sf::Text hint(*fontPtr, "Close window to return to menu", 12);
            hint.setPosition(sf::Vector2f(panelX, GRID_H * CELL_SIZE - 30));
            hint.setFillColor(sf::Color(150, 150, 150));
            heatWindow.draw(hint);
        }

        heatWindow.display();
    }
}

int main() {
    const int GRID_W = 40;
    const int GRID_H = 28;
    const int CELL_SIZE = 20;

    sf::RenderWindow menuWindow(sf::VideoMode(sf::Vector2u(800, 600)), "Maze - Main Menu");
    menuWindow.setFramerateLimit(60);

    Menu menu;
    const sf::Font *fontPtr = menu.isFontLoaded() ? &menu.getFont() : nullptr;

    // LOOP INFINITO
    while (menuWindow.isOpen()) {
        // Menú 1: Modo
        std::vector<std::string> modeOptions = {
            "Classic Mode",
            "Collector Mode (3 Treasures)",
            "Algorithm Ranking (AVL Tree)",
            "Exploration Heatmap (Sparse Matrix)"
        };
        Menu modeMenu(modeOptions, "Select Game Mode");
        int modeChoice = modeMenu.run(menuWindow);
        if (modeChoice < 0) break;

        // Opciones especiales: Ranking y Heatmap
        if (modeChoice == 2) {
            runAlgorithmRanking(menuWindow, fontPtr);
            continue;
        }
        if (modeChoice == 3) {
            showHeatmapVisualization(menuWindow, fontPtr);
            continue;
        }

        bool isCollectorMode = (modeChoice == 1);

        // Menú 2: Algoritmo generación
        int choice = menu.run(menuWindow);
        if (choice < 0) continue;

        Grid grid(GRID_W, GRID_H);
        std::srand(static_cast<unsigned>(std::time(nullptr)));

        std::unique_ptr<ChallengeSystem> challenges;
        if (isCollectorMode) {
            challenges = std::make_unique<ChallengeSystem>(grid);
        }

        std::unique_ptr<MazeAlgorithm> algo;
        std::string title;

        Coord start, goal;
        // Todos los modos ahora empiezan en el centro y terminan en una esquina aleatoria
        start = Coord(GRID_W / 2, GRID_H / 2);
        goal = getRandomCorner(grid, start);

        if (isCollectorMode) {
            switch (choice) {
                case 0:
                    algo.reset(new DFSCollectorAlgorithm(grid, challenges.get()));
                    title = "DFS";
                    break;
                case 1:
                    algo.reset(new PrimsCollectorAlgorithm(grid, challenges.get()));
                    title = "Prim's";
                    break;
                case 2:
                    algo.reset(new HuntAndKillCollectorAlgorithm(grid, challenges.get()));
                    title = "Hunt&Kill";
                    break;
                case 3:
                    algo.reset(new KruskalsCollectorAlgorithm(grid, challenges.get()));
                    title = "Kruskal's";
                    break;
                default:
                    algo.reset(new DFSCollectorAlgorithm(grid, challenges.get()));
                    title = "DFS";
                    break;
            }
            
            // Menú 3: Estrategia
            std::vector<std::string> strategyOptions = {
                "A*",
                "Greedy",
                "UCS",
                "DFS"
            };
            Menu strategyMenu(strategyOptions, "Choose Pathfinding Strategy");
            int stratChoice = strategyMenu.run(menuWindow);
            if (stratChoice < 0) continue;
            
            SolverStrategy strategy;
            switch (stratChoice) {
                case 0: strategy = SolverStrategy::ASTAR; break;
                case 1: strategy = SolverStrategy::GREEDY; break;
                case 2: strategy = SolverStrategy::UCS; break;
                case 3: strategy = SolverStrategy::DFS; break;
                default: strategy = SolverStrategy::ASTAR;
            }
            
            runCollectorMode(grid, *algo, *challenges, start, goal, CELL_SIZE, title, strategy, fontPtr);
        } else {
            switch (choice) {
                case 0:
                    algo.reset(new DFSAlgorithm(grid));
                    title = "DFS";
                    break;
                case 1:
                    algo.reset(new PrimsAlgorithm(grid));
                    title = "Prim's";
                    break;
                case 2:
                    algo.reset(new HuntAndKillAlgorithm(grid));
                    title = "Hunt&Kill";
                    break;
                case 3:
                    algo.reset(new KruskalsAlgorithm(grid));
                    title = "Kruskal's";
                    break;
                default:
                    algo.reset(new DFSAlgorithm(grid));
                    title = "DFS";
                    break;
            }
            
            runAlgorithm(grid, *algo, CELL_SIZE, title, fontPtr);
        }
        
        // Al cerrar ventana de juego, vuelve al inicio del loop
    }

    return 0;
}