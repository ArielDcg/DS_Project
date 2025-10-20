#include "Menu.h"
#include <iostream>

Menu::Menu() {
    options = {
        "DFS (Recursive Backtracker)",
        "Prim's (incremental)",
        "Kruskal's (placeholder)",
        "Recursive Division (placeholder)"
    };

    if (font.openFromFile("resources/DejaVuSans.ttf") ||
        font.openFromFile("DejaVuSans.ttf") ||
        font.openFromFile("arial.ttf")) {
        fontLoaded = true;
    } else {
        std::cerr << "Warning: couldn't load DejaVuSans.ttf or arial.ttf — menu text will be hidden.";
        fontLoaded = false;
    }
}

int Menu::run(sf::RenderWindow &window) {
    const int W = static_cast<int>(window.getSize().x);
    const int H = static_cast<int>(window.getSize().y);
    const int boxW = 480;
    const int boxH = 56;
    const int startY = (H / 2) - (static_cast<int>(options.size()) * (boxH + 12)) / 2;

    sf::Clock clk;
    sf::Time debounce = sf::milliseconds(160);
    sf::Time last = sf::Time::Zero;

    while (window.isOpen()) {
        while (auto evOpt = window.pollEvent()) {
            const sf::Event &ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) return -1;
        }

        sf::Time now = clk.getElapsedTime();
        if (now - last > debounce) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                selected = (selected - 1 + static_cast<int>(options.size())) % static_cast<int>(options.size());
                last = now;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                selected = (selected + 1) % static_cast<int>(options.size());
                last = now;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
                return selected;
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && now - last > debounce) {
            sf::Vector2i mp = sf::Mouse::getPosition(window);
            for (size_t i = 0; i < options.size(); ++i) {
                int bx = (W - boxW) / 2;
                int by = startY + static_cast<int>(i) * (boxH + 12);
                sf::IntRect r(sf::Vector2i(bx, by), sf::Vector2i(boxW, boxH));
                if (r.contains(mp)) {
                    last = now;
                    return static_cast<int>(i);
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));

        if (fontLoaded) {
            sf::Text title(font, "Choose Algorithm", 32);
            title.setPosition(sf::Vector2f(static_cast<float>(W / 2 - 160), static_cast<float>(startY - 96)));
            title.setFillColor(sf::Color::White);
            window.draw(title);
        }

        for (size_t i = 0; i < options.size(); ++i) {
            int bx = (W - boxW) / 2;
            int by = startY + static_cast<int>(i) * (boxH + 12);
            sf::RectangleShape box(sf::Vector2f(static_cast<float>(boxW), static_cast<float>(boxH)));
            box.setPosition(sf::Vector2f(static_cast<float>(bx), static_cast<float>(by)));
            if (static_cast<int>(i) == selected) box.setFillColor(sf::Color(60, 120, 220));
            else box.setFillColor(sf::Color(80, 80, 80));
            window.draw(box);

            if (fontLoaded) {
                sf::Text t(font, options[i], 22);
                t.setPosition(sf::Vector2f(static_cast<float>(bx + 16), static_cast<float>(by + boxH / 2 - 12)));
                t.setFillColor(sf::Color::White);
                window.draw(t);
            }
        }

        if (fontLoaded) {
            sf::Text hint(font, "Use Up/Down + Enter, or click a box", 16);
            hint.setPosition(sf::Vector2f(static_cast<float>(W / 2 - 160), static_cast<float>(startY + static_cast<int>(options.size()) * (boxH + 12) + 12)));
            hint.setFillColor(sf::Color(200, 200, 200));
            window.draw(hint);
        }

        window.display();
    }

    return -1;
}