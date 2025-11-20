// include/Menu.h
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Menu {
public:
    Menu();
    Menu(const std::vector<std::string>& options, const std::string& title = "Choose Algorithm");

    int run(sf::RenderWindow &window);

    // New: safe public accessors for font & fontLoaded
    bool isFontLoaded() const { return fontLoaded; }
    const sf::Font &getFont() const { return font; }

private:
    std::vector<std::string> options;
    std::string title;
    int selected = 0;
    bool fontLoaded = false;
    sf::Font font;
};
(necesario para unordered_map)