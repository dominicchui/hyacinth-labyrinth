#pragma once

#include <vector>
#include <string>

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
    Cell(CellType _type, char _wallRep, char _pathRep, char _closedRep):
        type(_type), WALL_REPRESENTATION(_wallRep), PATH_REPRESENTATION(_pathRep), CLOSED_AREA_REPRESENTATION(_closedRep){};

    CellType type;
    bool eastOpen = false;
    bool westOpen = false;
    bool northOpen = false;
    bool southOpen = false;
    Direction exitDir;

    std::string toString();

    void assignStringRepresentations(char wall, char path, char closed);
private:
    char WALL_REPRESENTATION = ' ';
    char PATH_REPRESENTATION = 'O';
    char CLOSED_AREA_REPRESENTATION = 'C';

};

