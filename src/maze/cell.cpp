#include "cell.h"
#include <iostream>

Cell::Cell()
{

}

std::string Cell::toString() {
    if (type == CellType::Open) {
        // temp debug
        switch (exitDir) {
        case Direction::N:
            return "N";
        case Direction::E:
            return "E";
        case Direction::S:
            return "S";
        case Direction::W:
            return "W";
        }
//        return "O";
        return {PATH_REPRESENTATION};
    } else if (type == CellType::Closed) {
        return {CLOSED_AREA_REPRESENTATION};
    } else {
        return {WALL_REPRESENTATION};
    }
};

void Cell::assignStringRepresentations(char wall, char path, char closed) {
    WALL_REPRESENTATION = wall;
    PATH_REPRESENTATION = path;
    CLOSED_AREA_REPRESENTATION = closed;
}
