#pragma once
#include "Grid.h"

struct MazeAlgorithm {
    virtual ~MazeAlgorithm() = default;
    // perform one small action; return true when finished
    virtual bool step() = 0;
    virtual bool finished() const = 0;
    // optional: provide a cell to highlight (for UI)
    virtual bool getCurrent(Coord &out) const { (void)out; return false; }
};