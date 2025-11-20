#pragma once
#include <functional>

struct Coord;

struct CoordHash {
    std::size_t operator()(const Coord& c) const {
        std::size_t h1 = std::hash<int>{}(c.x);
        std::size_t h2 = std::hash<int>{}(c.y);
        return h1 ^ (h2 << 1);
    }
};