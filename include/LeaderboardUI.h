#pragma once
#include "RankingTree.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>

class LeaderboardUI {
public:
    explicit LeaderboardUI(const std::string& fontPath = "assets/arial.ttf") {
        font_.loadFromFile(fontPath);
        title_.setFont(font_);
        title_.setCharacterSize(16);
        title_.setFillColor(sf::Color::Yellow);
        title_.setString("Leaderboard");
        title_.setPosition(10.f, 10.f);
    }

    void draw(sf::RenderTarget& target, const RankingTree& ranking, int top = 5) {
        target.draw(title_);
        auto topStats = ranking.topK(top);
        float y = 35.f;
        for (int i = 0; i < (int)topStats.size(); ++i) {
            const auto& s = topStats[i];
            std::ostringstream oss;
            oss << (i+1) << ". " << s.name
                << " | runs: " << s.runs
                << " | avgT: " << std::fixed << std::setprecision(2) << s.avgTimeMs() << " ms"
                << " | avgN: " << std::fixed << std::setprecision(1) << s.avgNodes()
                << " | eff: " << std::fixed << std::setprecision(2) << s.efficiencyScore();
            sf::Text t(oss.str(), font_, 14);
            t.setPosition(10.f, y);
            t.setFillColor(sf::Color::White);
            target.draw(t);
            y += 20.f;
        }
    }

private:
    sf::Font font_;
    sf::Text title_;
};
