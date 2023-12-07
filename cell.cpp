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
    } else if (type == CellType::Closed) {
        return "C";
    } else {
        return " ";
    }
};
