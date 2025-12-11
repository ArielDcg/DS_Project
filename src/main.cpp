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
#include "OriginShiftMaze.h"
#include "PortalSystem.h"
#include "GraphAnalysis.h"

#include "DFSAlgorithm.cpp"
#include "PrimsAlgorithm.cpp"
#include "HuntAndKillAlgorithm.cpp"
#include "KruskalsAlgorithm.cpp"
#include "CollectorMazeGen.cpp"
#include "OriginShiftMaze.cpp"
#include "PortalSystem.cpp"
#include "GraphAnalysis.cpp"

#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <chrono>
#include <iostream>

void displayHeatmap(const Grid& grid, const ExplorationHeatmap& heatmap,
                   const sf::Font* fontPtr, const std::string& windowTitle);

// Forward declaration for user vs computer mode
void runUserVsComputer(int gridW, int gridH, const std::string &title,
                       SolverStrategy strategy, sf::Time solverStepTime);

// Tamaño global del laberinto (modificar aquí para cambiar todas las dimensiones)
const int GRID_W = 40;
const int GRID_H = 28;

// --- CYBERPUNK PALETTE CONSTANTS ---
const sf::Color CP_BG(10, 5, 20);            // Dark Violet/Black
const sf::Color CP_WALL(0, 200, 255);        // Neon Blue
const sf::Color CP_USER(255, 0, 110);        // Neon Pink/Red
const sf::Color CP_SOLVER(180, 50, 255);     // Neon Purple
const sf::Color CP_TRACE_USER(255, 0, 110, 100);
const sf::Color CP_TRACE_SOLVER(180, 50, 255, 100);
const sf::Color CP_SOLUTION(0, 255, 0);      // Neon Green
const sf::Color CP_GOAL(255, 50, 50);        // Red Neon

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

// --- RUN COLLECTOR MODE ---
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
                return;
            }

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

        // Treasures
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

        // Start/Goal
        sf::CircleShape startMarker(cellSize * 0.4f);
        startMarker.setPosition(sf::Vector2f(start.x * cellSize + cellSize * 0.1f, start.y * cellSize + cellSize * 0.1f));
        startMarker.setFillColor(sf::Color(0, 255, 0, 150));
        window.draw(startMarker);

        sf::CircleShape goalMarker(cellSize * 0.4f);
        goalMarker.setPosition(sf::Vector2f(goal.x * cellSize + cellSize * 0.1f, goal.y * cellSize + cellSize * 0.1f));
        goalMarker.setFillColor(sf::Color(255, 0, 0, 150));
        window.draw(goalMarker);

        // Generation Cursor
        Coord genCur;
        if (!algo.finished() && algo.getCurrent(genCur)) {
            sf::RectangleShape curRect(sf::Vector2f((float)cellSize, (float)cellSize));
            curRect.setPosition(sf::Vector2f((float)genCur.x * cellSize, (float)genCur.y * cellSize));
            curRect.setFillColor(sf::Color(0, 160, 0, 140));
            window.draw(curRect);
        }

        // Solver Render
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
                    } else if (states[x][y] == CollectorSolver::OPEN) {
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

            // Path Segments
            const std::vector<PathSegment>& segments = solver->getSegments();
            for (const PathSegment& seg : segments) {
                ObjectiveColors segColors = getColorsForObjective(seg.objectiveIndex);
                for (const Coord& c : seg.path) {
                    sf::RectangleShape rect(sf::Vector2f((float)cellSize, (float)cellSize));
                    rect.setPosition(sf::Vector2f((float)c.x * cellSize, (float)c.y * cellSize));
                    rect.setFillColor(segColors.path);
                    window.draw(rect);
                }
                if (seg.foundBonus) {
                    sf::CircleShape bonus(cellSize * 0.4f);
                    bonus.setPosition(sf::Vector2f(seg.bonusTreasure.x * cellSize + cellSize * 0.1f, 
                                                    seg.bonusTreasure.y * cellSize + cellSize * 0.1f));
                    bonus.setFillColor(sf::Color(0, 255, 0, 200));
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

        // Overlay text
        if (fontPtr) {
            sf::Text titleText(*fontPtr, title + " - " + getStrategyName(strategy), 16);
            titleText.setPosition(sf::Vector2f(6.f, 6.f));
            titleText.setFillColor(sf::Color(235,235,235,230));
            window.draw(titleText);
            
            if (solverInitialized && solver) {
                 std::string treasureText = "Treasures: " + std::to_string(solver->getTreasuresCollected()) + "/3";
                 sf::Text tText(*fontPtr, treasureText, 14);
                 tText.setPosition(sf::Vector2f(6.f, 26.f));
                 tText.setFillColor(sf::Color(255, 215, 0));
                 window.draw(tText);
                 
                 // Hint at the end
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

// --- RUN USER VS COMPUTER ---
void runUserVsComputer(int gridW, int gridH, const std::string &title,
                       SolverStrategy strategy, sf::Time solverStepTime) {
    // Calculate Cell Size
    float maxWinW = 1800.0f;
    float maxWinH = 950.0f;
    float cellByW = (maxWinW - 40.0f) / (2.0f * (float)gridW);
    float cellByH = (maxWinH - 40.0f) / (float)gridH;
    int cellSize = static_cast<int>(std::min(cellByW, cellByH));
    if (cellSize < 5) cellSize = 5;

    // Generate Maze
    Grid grid(gridW, gridH);
    DFSAlgorithm generator(grid);
    while(!generator.finished()) {
        generator.step();
    }
    Coord start(0, 0);
    Coord goal(gridW - 1, gridH - 1);

    unsigned int winW = static_cast<unsigned int>(grid.width() * cellSize * 2 + 20); 
    unsigned int winH = static_cast<unsigned int>(grid.height() * cellSize);
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(winW, winH)), title);
    window.setFramerateLimit(60);

    // Assets
    sf::Texture wastedTexture, duckTexture;
    bool wastedLoaded = wastedTexture.loadFromFile("assets/wasted.png");
    bool duckLoaded = duckTexture.loadFromFile("assets/duck.png");

    sf::Sprite wastedSprite(wastedTexture);
    sf::Sprite duckSprite(duckTexture);
    
    if (wastedLoaded) {
        float scale = (float)winW * 0.8f / wastedTexture.getSize().x;
        wastedSprite.setScale(sf::Vector2f(scale, scale));
        wastedSprite.setOrigin(sf::Vector2f(wastedTexture.getSize().x / 2.0f, wastedTexture.getSize().y / 2.0f));
        wastedSprite.setPosition(sf::Vector2f(winW / 2.0f, winH / 2.0f));
    }
    if (duckLoaded) {
        float scale = (float)winH * 0.6f / duckTexture.getSize().y;
        duckSprite.setScale(sf::Vector2f(scale, scale));
        duckSprite.setOrigin(sf::Vector2f(duckTexture.getSize().x / 2.0f, duckTexture.getSize().y / 2.0f));
        duckSprite.setPosition(sf::Vector2f(winW / 2.0f, winH / 2.0f));
    }

    // USER STATE
    Coord userPos = start;
    bool userWon = false;
    sf::Clock inputClock;
    sf::Time inputDebounce = sf::milliseconds(80);

    // USER TRACE
    std::vector<std::vector<bool>> userVisited(grid.width(), std::vector<bool>(grid.height(), false));
    userVisited[start.x][start.y] = true;

    // SOLVER STATE
    std::unique_ptr<AStarSolver> aStar;
    std::unique_ptr<GreedySolver> greedy;
    std::unique_ptr<UCSSolver> ucs;
    std::unique_ptr<MazeSolver> dfs;

    switch (strategy) {
        case SolverStrategy::ASTAR:
            aStar = std::make_unique<AStarSolver>(grid, start, goal);
            break;
        case SolverStrategy::GREEDY:
            greedy = std::make_unique<GreedySolver>(grid, start, goal);
            break;
        case SolverStrategy::UCS:
            ucs = std::make_unique<UCSSolver>(grid, start, goal);
            break;
        case SolverStrategy::DFS:
        default:
            dfs = std::make_unique<MazeSolver>(grid, start, goal);
            break;
    }

    sf::Clock solverClock;
    sf::Time solverTimer = sf::Time::Zero;
    bool solverWon = false;

    // Helper functions
    auto solverFinished = [&]() {
        if (aStar) return aStar->finished();
        if (greedy) return greedy->finished();
        if (ucs) return ucs->finished();
        if (dfs) return dfs->finished();
        return true; 
    };

    auto solverStep = [&]() {
        if (aStar) return aStar->step();
        if (greedy) return greedy->step();
        if (ucs) return ucs->step();
        if (dfs) return dfs->step();
        return false;
    };

    auto solverCurrent = [&](Coord &out) {
        if (aStar) return aStar->getCurrent(out);
        if (greedy) return greedy->getCurrent(out);
        if (ucs) return ucs->getCurrent(out);
        if (dfs) return dfs->getCurrent(out);
        out = start;
        return true;
    };

    auto solverGetSolution = [&]() -> const std::vector<Coord>& {
        static const std::vector<Coord> empty;
        if (aStar) return aStar->getSolution();
        if (greedy) return greedy->getSolution();
        if (ucs) return ucs->getSolution();
        if (dfs) return dfs->getSolution();
        return empty;
    };

    auto drawSolverTrace = [&](float offsetX) {
        if (dfs) {
            const auto& visited = dfs->getVisited();
            for (int y = 0; y < grid.height(); ++y) {
                for (int x = 0; x < grid.width(); ++x) {
                    if (visited[x][y]) {
                        sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                        r.setPosition(sf::Vector2f(offsetX + x * cellSize, y * cellSize));
                        r.setFillColor(CP_TRACE_SOLVER);
                        window.draw(r);
                    }
                }
            }
        } else {
            auto drawGrid = [&](const auto& solverPtr) {
                const auto& states = solverPtr->getStateGrid();
                for (int y = 0; y < grid.height(); ++y) {
                    for (int x = 0; x < grid.width(); ++x) {
                        if (states[x][y] == 2) { // CLOSED
                            sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                            r.setPosition(sf::Vector2f(offsetX + x * cellSize, y * cellSize));
                            r.setFillColor(CP_TRACE_SOLVER); 
                            window.draw(r);
                        }
                    }
                }
            };
            if (aStar) drawGrid(aStar);
            else if (greedy) drawGrid(greedy);
            else if (ucs) drawGrid(ucs);
        }
    };

    auto drawLine = [&](sf::RenderTarget &target, float x1, float y1, float x2, float y2, const sf::Color &col) {
        sf::Vertex line[] = {
            {sf::Vector2f(x1, y1), col},
            {sf::Vector2f(x2, y2), col}
        };
        target.draw(line, 2, sf::PrimitiveType::Lines);
    };

    auto drawMaze = [&](float offsetX, const Coord& playerPos, const sf::Color& playerColor, bool isUser) {
        // Trace
        if (isUser) {
            for (int y = 0; y < grid.height(); ++y) {
                for (int x = 0; x < grid.width(); ++x) {
                    if (userVisited[x][y]) {
                        sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                        r.setPosition(sf::Vector2f(offsetX + x * cellSize, y * cellSize));
                        r.setFillColor(CP_TRACE_USER); 
                        window.draw(r);
                    }
                }
            }
        } else {
            drawSolverTrace(offsetX);
        }

        // Walls
        for (int y = 0; y < grid.height(); ++y) {
            for (int x = 0; x < grid.width(); ++x) {
                float xpos = offsetX + x * cellSize;
                float ypos = y * cellSize;

                if (grid.at(x, y).walls[0]) drawLine(window, xpos, ypos, xpos + cellSize, ypos, CP_WALL);
                if (grid.at(x, y).walls[1]) drawLine(window, xpos, ypos, xpos, ypos + cellSize, CP_WALL);
                if (grid.at(x, y).walls[2]) drawLine(window, xpos + cellSize, ypos, xpos + cellSize, ypos + cellSize, CP_WALL);
                if (grid.at(x, y).walls[3]) drawLine(window, xpos, ypos + cellSize, xpos + cellSize, ypos + cellSize, CP_WALL);
            }
        }
        
        // Solution Path
        if (!isUser && solverWon) {
             const auto& sol = solverGetSolution();
             if (sol.size() > 1) {
                 for (size_t i = 0; i < sol.size() - 1; ++i) {
                     float x1 = offsetX + sol[i].x * cellSize + cellSize / 2.0f;
                     float y1 = sol[i].y * cellSize + cellSize / 2.0f;
                     float x2 = offsetX + sol[i+1].x * cellSize + cellSize / 2.0f;
                     float y2 = sol[i+1].y * cellSize + cellSize / 2.0f;
                     
                     sf::Vertex line[] = {
                         {sf::Vector2f(x1, y1), CP_SOLUTION},
                         {sf::Vector2f(x2, y2), CP_SOLUTION}
                     };
                     window.draw(line, 2, sf::PrimitiveType::Lines);
                 }
             }
        }
        
        // Goal
        sf::RectangleShape goalRect(sf::Vector2f(cellSize * 0.6f, cellSize * 0.6f));
        goalRect.setPosition(sf::Vector2f(offsetX + goal.x * cellSize + cellSize * 0.2f, goal.y * cellSize + cellSize * 0.2f));
        goalRect.setFillColor(CP_GOAL);
        window.draw(goalRect);

        // Player/Solver
        sf::RectangleShape pRect(sf::Vector2f(cellSize * 0.8f, cellSize * 0.8f));
        pRect.setPosition(sf::Vector2f(offsetX + playerPos.x * cellSize + cellSize * 0.1f, playerPos.y * cellSize + cellSize * 0.1f));
        pRect.setFillColor(playerColor);
        window.draw(pRect);
    };

    while (window.isOpen()) {
        while (auto evOpt = window.pollEvent()) {
            const sf::Event &ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) {
                window.close();
                return;
            }
        }

        if (userWon || solverWon) {
             if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                 window.close();
                 return;
             }
        } else {
            if (inputClock.getElapsedTime() > inputDebounce) {
                int dx = 0, dy = 0;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dy = -1;
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dy = 1;
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dx = -1;
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dx = 1;

                if (dx != 0 || dy != 0) {
                    bool blocked = false;
                    const auto& cell = grid.at(userPos.x, userPos.y);
                    if (dy == -1 && cell.walls[0]) blocked = true;
                    if (dx == 1 && cell.walls[2]) blocked = true;
                    if (dy == 1 && cell.walls[3]) blocked = true;
                    if (dx == -1 && cell.walls[1]) blocked = true;

                    if (!blocked) {
                        userPos.x += dx;
                        userPos.y += dy;
                        userVisited[userPos.x][userPos.y] = true;
                        inputClock.restart();
                        if (userPos.x == goal.x && userPos.y == goal.y) userWon = true;
                    }
                }
            }

            solverTimer += solverClock.restart();
            if (solverTimer >= solverStepTime) {
                if (!solverFinished()) solverStep();
                solverTimer = sf::Time::Zero;
                if (solverFinished()) {
                    const auto& sol = solverGetSolution();
                    if (!sol.empty() && sol.back().x == goal.x && sol.back().y == goal.y) solverWon = true;
                }
                Coord solverPos;
                if (solverCurrent(solverPos) && solverPos.x == goal.x && solverPos.y == goal.y) solverWon = true;
            }
        }

        window.clear(CP_BG);

        // Draw Left (User)
        drawMaze(0, userPos, CP_USER, true);

        // Separator
        sf::RectangleShape sep(sf::Vector2f(20.0f, (float)winH));
        sep.setPosition(sf::Vector2f((float)grid.width() * cellSize, 0.0f));
        sep.setFillColor(sf::Color(50, 50, 100)); // Darker separator
        window.draw(sep);

        // Draw Right (Solver)
        Coord solverCur = start;
        solverCurrent(solverCur);
        drawMaze((float)grid.width() * cellSize + 20.0f, solverCur, CP_SOLVER, false);

        if (userWon) {
            if (duckLoaded) window.draw(duckSprite);
            else {
                 sf::RectangleShape winOverlay(sf::Vector2f(300.0f, 100.0f));
                 winOverlay.setPosition(sf::Vector2f(winW/2.0f - 150.0f, winH/2.0f - 50.0f));
                 winOverlay.setFillColor(sf::Color(0, 200, 0, 200));
                 window.draw(winOverlay);
            }
        } else if (solverWon) {
            if (wastedLoaded) window.draw(wastedSprite);
            else {
                 sf::RectangleShape loseOverlay(sf::Vector2f(300.0f, 100.0f));
                 loseOverlay.setPosition(sf::Vector2f(winW/2.0f - 150.0f, winH/2.0f - 50.0f));
                 loseOverlay.setFillColor(sf::Color(200, 0, 0, 200));
                 window.draw(loseOverlay);
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
            sf::Text header(*fontPtr, "Rank  Solver Algorithm           Score    Nodes   Time(ms)  Treasures", 14);
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
                "Score = 1000 - (nodos*0.5) - (tiempo*500) + (tesoros*50)",
                "Nodes = Nodos expandidos (celdas marcadas como CLOSED)",
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


// ===================================
// ORIGIN SHIFT MODE - Laberinto Viviente
// ===================================
void runOriginShiftMode(Grid& grid, MazeAlgorithm& algo, int cellSize, 
                        const std::string& title, SolverStrategy strategy,
                        const sf::Font* fontPtr = nullptr) {
    unsigned int winW = static_cast<unsigned int>(grid.width() * cellSize);
    unsigned int winH = static_cast<unsigned int>(grid.height() * cellSize);
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(winW, winH)), 
                           title + " - ORIGIN SHIFT [" + getStrategyName(strategy) + "]");
    window.setFramerateLimit(60);

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time stepTime = sf::milliseconds(8);
    sf::Time originShiftTime = sf::milliseconds(200); // Velocidad del Origin Shift
    sf::Time originShiftAccum = sf::Time::Zero;
    
    bool mazeGenerated = false;
    std::unique_ptr<OriginShiftMaze> originShift;
    std::unique_ptr<MazeSolver> solver;
    bool solverStarted = false;
    
    Coord start(grid.width() / 2, grid.height() / 2);
    Coord goal = getRandomCorner(grid, start);

    auto drawLine = [&](sf::RenderTarget& target, float x1, float y1, float x2, float y2, const sf::Color& col) {
        sf::Vertex verts[2];
        verts[0].position = sf::Vector2f(x1, y1);
        verts[0].color = col;
        verts[1].position = sf::Vector2f(x2, y2);
        verts[1].color = col;
        target.draw(verts, 2, sf::PrimitiveType::Lines);
    };

    while (window.isOpen()) {
        while (auto evOpt = window.pollEvent()) {
            const sf::Event& ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) {
                window.close();
                return;
            }
        }

        sf::Time dt = clock.restart();
        accumulator += dt;
        
        while (accumulator >= stepTime) {
            if (!algo.finished()) {
                algo.step();
            } else if (!mazeGenerated) {
                // Inicializar Origin Shift después de generar el laberinto
                originShift = std::make_unique<OriginShiftMaze>(grid);
                originShift->initializeFromMaze();
                mazeGenerated = true;
            } else if (!solverStarted) {
                // Iniciar solver
                solver = std::make_unique<MazeSolver>(grid, start, goal);
                solverStarted = true;
            } else if (!solver->finished()) {
                solver->step();
            }
            accumulator -= stepTime;
        }
        
        // Origin Shift: cambiar raíz periódicamente
        if (mazeGenerated && originShift) {
            originShiftAccum += dt;
            while (originShiftAccum >= originShiftTime) {
                originShift->update();
                originShift->applyToGrid();
                originShiftAccum -= originShiftTime;
            }
        }

        window.clear(sf::Color::Black);

        // Dibujar grid
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

                // Paredes
                if (grid.at(x, y).walls[0]) drawLine(window, xpos, ypos, xpos + cellSize, ypos, sf::Color(220,220,220));
                if (grid.at(x, y).walls[1]) drawLine(window, xpos, ypos, xpos, ypos + cellSize, sf::Color(220,220,220));
                if (grid.at(x, y).walls[2]) drawLine(window, xpos + cellSize, ypos, xpos + cellSize, ypos + cellSize, sf::Color(220,220,220));
                if (grid.at(x, y).walls[3]) drawLine(window, xpos, ypos + cellSize, xpos + cellSize, ypos + cellSize, sf::Color(220,220,220));
            }
        }
        
        // Dibujar TODAS las raíces del Origin Shift (cada una con color diferente)
        if (originShift) {
            const auto& roots = originShift->getRoots();
            std::vector<sf::Color> rootColors = {
                sf::Color(255, 100, 255, 200),  // Magenta
                sf::Color(100, 255, 255, 200),  // Cyan
                sf::Color(255, 255, 100, 200)   // Amarillo
            };
            
            for (size_t i = 0; i < roots.size(); ++i) {
                const Coord& root = roots[i];
                sf::Color color = rootColors[i % rootColors.size()];
                
                sf::CircleShape rootMarker(cellSize * 0.3f);
                rootMarker.setPosition(sf::Vector2f(root.x * cellSize + cellSize * 0.2f, 
                                                     root.y * cellSize + cellSize * 0.2f));
                rootMarker.setFillColor(color);
                rootMarker.setOutlineThickness(2.0f);
                rootMarker.setOutlineColor(sf::Color::White);
                window.draw(rootMarker);
            }
        }

        // Start/Goal
        sf::CircleShape startMarker(cellSize * 0.4f);
        startMarker.setPosition(sf::Vector2f(start.x * cellSize + cellSize * 0.1f, start.y * cellSize + cellSize * 0.1f));
        startMarker.setFillColor(sf::Color(0, 255, 0, 150));
        window.draw(startMarker);

        sf::CircleShape goalMarker(cellSize * 0.4f);
        goalMarker.setPosition(sf::Vector2f(goal.x * cellSize + cellSize * 0.1f, goal.y * cellSize + cellSize * 0.1f));
        goalMarker.setFillColor(sf::Color(255, 0, 0, 150));
        window.draw(goalMarker);
        
        // Solver path
        if (solver) {
            const auto& visited = solver->getVisited();
            for (int y = 0; y < grid.height(); ++y) {
                for (int x = 0; x < grid.width(); ++x) {
                    if (visited[x][y]) {
                        sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                        r.setPosition(sf::Vector2f(x * cellSize, y * cellSize));
                        r.setFillColor(sf::Color(120, 80, 160, 100));
                        window.draw(r);
                    }
                }
            }
            
            Coord cur;
            if (solver->getCurrent(cur)) {
                sf::RectangleShape s(sf::Vector2f((float)cellSize, (float)cellSize));
                s.setPosition(sf::Vector2f((float)cur.x * cellSize, (float)cur.y * cellSize));
                s.setFillColor(sf::Color(255, 200, 0, 170));
                window.draw(s);
            }
        }

        // Overlay con información del GRAFO
        if (fontPtr) {
            sf::Text titleText(*fontPtr, "ORIGIN SHIFT - Living Maze (Graph Structure)", 16);
            titleText.setPosition(sf::Vector2f(6.f, 6.f));
            titleText.setFillColor(sf::Color(255, 100, 255));
            window.draw(titleText);
            
            if (originShift) {
                std::string graphInfo = "Graph: " + std::to_string(originShift->getNodeCount()) + " nodes, " 
                                      + std::to_string(originShift->getEdgeCount()) + " edges, "
                                      + std::to_string(originShift->getRoots().size()) + " active roots";
                sf::Text infoText(*fontPtr, graphInfo, 12);
                infoText.setPosition(sf::Vector2f(6.f, 26.f));
                infoText.setFillColor(sf::Color(100, 255, 255));
                window.draw(infoText);
            }
        }
        
        window.display();
    }
}

// ===================================
// PORTAL JUMPER MODE
// ===================================
void runPortalMode(Grid& grid, MazeAlgorithm& algo, int cellSize,
                   const std::string& title, SolverStrategy strategy,
                   const sf::Font* fontPtr = nullptr) {
    unsigned int winW = static_cast<unsigned int>(grid.width() * cellSize);
    unsigned int winH = static_cast<unsigned int>(grid.height() * cellSize);
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(winW, winH)), 
                           title + " - PORTAL JUMPER [" + getStrategyName(strategy) + "]");
    window.setFramerateLimit(60);

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time stepTime = sf::milliseconds(8);
    
    Coord start(grid.width() / 2, grid.height() / 2);
    Coord goal = getRandomCorner(grid, start);
    
    std::unique_ptr<PortalSystem> portals;
    std::unique_ptr<MazeSolver> solver;
    bool mazeGenerated = false;
    bool solverStarted = false;
    
    // Para análisis de grafos
    std::unique_ptr<GraphAnalysis> graphAnalysis;
    GraphStats graphStats;
    bool diameterCalculated = false;

    auto drawLine = [&](sf::RenderTarget& target, float x1, float y1, float x2, float y2, const sf::Color& col) {
        sf::Vertex verts[2];
        verts[0].position = sf::Vector2f(x1, y1);
        verts[0].color = col;
        verts[1].position = sf::Vector2f(x2, y2);
        verts[1].color = col;
        target.draw(verts, 2, sf::PrimitiveType::Lines);
    };

    while (window.isOpen()) {
        while (auto evOpt = window.pollEvent()) {
            const sf::Event& ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) {
                window.close();
                return;
            }
        }

        accumulator += clock.restart();
        while (accumulator >= stepTime) {
            if (!algo.finished()) {
                algo.step();
            } else if (!mazeGenerated) {
                // Generar portales y calcular diámetro
                portals = std::make_unique<PortalSystem>(grid.width(), grid.height());
                portals->generatePortals(start, goal, 2);
                
                graphAnalysis = std::make_unique<GraphAnalysis>(grid);
                graphAnalysis->buildAdjacencyList();
                graphStats = graphAnalysis->calculateDiameter();
                diameterCalculated = true;
                
                mazeGenerated = true;
            } else if (!solverStarted) {
                solver = std::make_unique<MazeSolver>(grid, start, goal);
                solverStarted = true;
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
        
        // Dibujar diámetro (Cyan)
        if (diameterCalculated && graphStats.path.size() > 1) {
            for (size_t i = 0; i < graphStats.path.size() - 1; ++i) {
                const Coord& a = graphStats.path[i];
                const Coord& b = graphStats.path[i + 1];
                float ax = a.x * cellSize + cellSize * 0.5f;
                float ay = a.y * cellSize + cellSize * 0.5f;
                float bx = b.x * cellSize + cellSize * 0.5f;
                float by = b.y * cellSize + cellSize * 0.5f;
                drawLine(window, ax, ay, bx, by, sf::Color(0, 255, 255, 180));
            }
        }
        
        // Portales (Verde brillante)
        if (portals) {
            for (const Portal& p : portals->getPortals()) {
                // Portal A
                sf::RectangleShape pa(sf::Vector2f((float)cellSize, (float)cellSize));
                pa.setPosition(sf::Vector2f(p.a.x * cellSize, p.a.y * cellSize));
                pa.setFillColor(sf::Color(0, 255, 100, 150));
                pa.setOutlineThickness(2.0f);
                pa.setOutlineColor(sf::Color(0, 255, 0));
                window.draw(pa);
                
                // Portal B
                sf::RectangleShape pb(sf::Vector2f((float)cellSize, (float)cellSize));
                pb.setPosition(sf::Vector2f(p.b.x * cellSize, p.b.y * cellSize));
                pb.setFillColor(sf::Color(0, 255, 100, 150));
                pb.setOutlineThickness(2.0f);
                pb.setOutlineColor(sf::Color(0, 255, 0));
                window.draw(pb);
                
                // Línea conectando portales
                float ax = p.a.x * cellSize + cellSize * 0.5f;
                float ay = p.a.y * cellSize + cellSize * 0.5f;
                float bx = p.b.x * cellSize + cellSize * 0.5f;
                float by = p.b.y * cellSize + cellSize * 0.5f;
                drawLine(window, ax, ay, bx, by, sf::Color(0, 255, 0, 100));
            }
        }

        // Start/Goal
        sf::CircleShape startMarker(cellSize * 0.4f);
        startMarker.setPosition(sf::Vector2f(start.x * cellSize + cellSize * 0.1f, start.y * cellSize + cellSize * 0.1f));
        startMarker.setFillColor(sf::Color(0, 255, 0, 200));
        window.draw(startMarker);

        sf::CircleShape goalMarker(cellSize * 0.4f);
        goalMarker.setPosition(sf::Vector2f(goal.x * cellSize + cellSize * 0.1f, goal.y * cellSize + cellSize * 0.1f));
        goalMarker.setFillColor(sf::Color(255, 0, 0, 200));
        window.draw(goalMarker);
        
        // Solver
        if (solver) {
            const auto& visited = solver->getVisited();
            for (int y = 0; y < grid.height(); ++y) {
                for (int x = 0; x < grid.width(); ++x) {
                    if (visited[x][y]) {
                        sf::RectangleShape r(sf::Vector2f((float)cellSize, (float)cellSize));
                        r.setPosition(sf::Vector2f(x * cellSize, y * cellSize));
                        r.setFillColor(sf::Color(180, 50, 255, 80));
                        window.draw(r);
                    }
                }
            }
            
            Coord cur;
            if (solver->getCurrent(cur)) {
                sf::RectangleShape s(sf::Vector2f((float)cellSize, (float)cellSize));
                s.setPosition(sf::Vector2f((float)cur.x * cellSize, (float)cur.y * cellSize));
                s.setFillColor(sf::Color(255, 200, 0, 170));
                window.draw(s);
            }
        }

        // Overlay
        if (fontPtr) {
            sf::Text titleText(*fontPtr, "PORTAL JUMPER - Wormholes Active", 16);
            titleText.setPosition(sf::Vector2f(6.f, 6.f));
            titleText.setFillColor(sf::Color(0, 255, 100));
            window.draw(titleText);
            
            if (diameterCalculated) {
                std::string diamStr = "Graph Diameter: " + std::to_string(graphStats.diameter);
                sf::Text diamText(*fontPtr, diamStr, 12);
                diamText.setPosition(sf::Vector2f(6.f, 26.f));
                diamText.setFillColor(sf::Color(0, 255, 255));
                window.draw(diamText);
            }
        }
        
        window.display();
    }
}

int main() {
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
            "Exploration Heatmap (Sparse Matrix)",
            "User vs Solver Mode",
            "Origin Shift (Living Maze)"
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

        if (modeChoice == 4) {
             // Maze Size Selection
             std::vector<std::string> sizeOptions = { 
                 "Small (21x21)", 
                 "Medium (31x31)", 
                 "Large (51x51)", 
                 "Extra Large (71x71)" 
             };
             Menu sizeMenu(sizeOptions, "Select Maze Size");
             int sizeChoice = sizeMenu.run(menuWindow);
             if (sizeChoice < 0) continue;

             int gW = 21, gH = 21;
             switch (sizeChoice) {
                 case 0: gW = 21; gH = 21; break;
                 case 1: gW = 31; gH = 31; break;
                 case 2: gW = 51; gH = 51; break;
                 case 3: gW = 71; gH = 71; break;
                 default: gW = 31; gH = 31; break;
             }

             // Menu selection for Opponent Algorithm
             std::vector<std::string> opponentOptions = { "A*", "Greedy", "UCS", "DFS" };
             Menu opponentMenu(opponentOptions, "Select Opponent Algorithm");
             int oppChoice = opponentMenu.run(menuWindow);
             if (oppChoice < 0) continue;

             SolverStrategy strategy;
             switch(oppChoice) {
                 case 0: strategy = SolverStrategy::ASTAR; break;
                 case 1: strategy = SolverStrategy::GREEDY; break;
                 case 2: strategy = SolverStrategy::UCS; break;
                 case 3: strategy = SolverStrategy::DFS; break;
                 default: strategy = SolverStrategy::ASTAR; break;
             }

             // Menu selection for Difficulty
             std::vector<std::string> diffOptions = { "Easy (Slow)", "Medium (Normal)", "Hard (Fast)" };
             Menu diffMenu(diffOptions, "Select Difficulty");
             int diffChoice = diffMenu.run(menuWindow);
             if (diffChoice < 0) continue;

             sf::Time stepTime;
             switch(diffChoice) {
                 case 0: stepTime = sf::milliseconds(1800); break; // Easy
                 case 1: stepTime = sf::milliseconds(600); break;  // Medium
                 case 2: stepTime = sf::milliseconds(150); break;  // Hard
                 default: stepTime = sf::milliseconds(600); break;
             }

             runUserVsComputer(gW, gH, "User vs Solver", strategy, stepTime);
             continue;
        }

        // ===== ORIGIN SHIFT MODE (modeChoice == 5) =====
        if (modeChoice == 5) {
            // Menú generador
            int genChoice = menu.run(menuWindow);
            if (genChoice < 0) continue;
            
            Grid grid(GRID_W, GRID_H);
            std::unique_ptr<MazeAlgorithm> algo;
            std::string title;
            
            switch (genChoice) {
                case 0: algo.reset(new DFSAlgorithm(grid)); title = "DFS"; break;
                case 1: algo.reset(new PrimsAlgorithm(grid)); title = "Prim's"; break;
                case 2: algo.reset(new HuntAndKillAlgorithm(grid)); title = "Hunt&Kill"; break;
                case 3: algo.reset(new KruskalsAlgorithm(grid)); title = "Kruskal's"; break;
                default: algo.reset(new DFSAlgorithm(grid)); title = "DFS"; break;
            }
            
            // Menú solver
            std::vector<std::string> stratOpts = { "A*", "Greedy", "UCS", "DFS" };
            Menu stratMenu(stratOpts, "Choose Solver");
            int stratChoice = stratMenu.run(menuWindow);
            if (stratChoice < 0) continue;
            
            SolverStrategy strategy;
            switch (stratChoice) {
                case 0: strategy = SolverStrategy::ASTAR; break;
                case 1: strategy = SolverStrategy::GREEDY; break;
                case 2: strategy = SolverStrategy::UCS; break;
                case 3: strategy = SolverStrategy::DFS; break;
                default: strategy = SolverStrategy::ASTAR; break;
            }
            
            runOriginShiftMode(grid, *algo, CELL_SIZE, title, strategy, fontPtr);
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
             // Classic Algorithm
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