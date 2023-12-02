#pragma once

#include <vector>
enum CellType {
    Empty, // cell that has not been initalized with a value, de facto wall
    Open, // cell that is an "open" space, e.g. corridor
    Closed // cell that is marked as off limits
};

enum Direction {
    N,
    E,
    S,
    W
};

static std::vector<Direction> directions { Direction::N, Direction::E, Direction::S, Direction::W };

class Cell
{
public:
    Cell();
    Cell(CellType _type): type(_type){};

    CellType type;
    bool eastOpen = false;
    bool westOpen = false;
    bool northOpen = false;
    bool southOpen = false;
    Direction exitDir;

};

