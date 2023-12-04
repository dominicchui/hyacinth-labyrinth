#include "cell.h"
#include <iostream>

Cell::Cell()
{

}

std::string Cell::toString() {
    if (type == CellType::Open) {
        return "O";
    } else if (type == CellType::Closed) {
        return "C";
    } else {
        return "W";
    }
};
