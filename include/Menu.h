#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct Menu {
    std::vector<std::string> options;
    int selected = 0;
    bool fontLoaded = false;
    sf::Font font;

    Menu();
    // returns selected index or -1 on close
    int run(sf::RenderWindow &window);
};