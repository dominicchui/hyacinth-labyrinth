#include "mazeblock.h"
#include <iostream>


MazeBlock::MazeBlock()
{

}

MazeBlock::MazeBlock(int _width, int _height): width(_width), height(_height) {
    cells = std::vector(size(),Cell(CellType::Empty));
}

MazeBlock::MazeBlock(int _width, int _height, bool _insertClosedSpaces): width(_width), height(_height) {
    cells = std::vector(size(),Cell(CellType::Empty));
    if (_insertClosedSpaces) {
        insertClosedSpaces();
    }
}

int MazeBlock::getRandomEmptyCell() {
    if (mazeCells.size() == size()) { return -1; }
    // todo move for performance?
    static std::random_device rd;
    static std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, size()-1);
    // todo maybe don't rely on luck
    // idea: shuffle cells and iterate
    while (true) {
        int n = distrib(gen);
        // check cell is valid target
        if (!isCellInMaze(n) && cells[n].type==CellType::Empty) {
            return n;
        }
    }
}

// generates a maze using Wilson's algorithm
// https://en.wikipedia.org/wiki/Maze_generation_algorithm#Wilson's_algorithm
void MazeBlock::generate() {
    // add random cell to maze if no borders are defined
    if (!hasDefinedExternalBorderCells()) {
        int n = getRandomEmptyCell();
//        std::cout << "first: " << n << std::endl;
        cells[n].type = CellType::Open;
        mazeCells.insert(n);
    }

    while (mazeCells.size() + closedCellsCount < size()) {
//        std::cout << toString() << std::endl;
        performRandomWalk();
    }
}

void MazeBlock::performRandomWalk() {
    // pick random empty cell
    int initial = getRandomEmptyCell();
//    std::cout << "walk start: " << initial << std::endl;

    // set up random generation
    static std::random_device rd; // a seed source for the random number engine
    static std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, directions.size());

    // FIRST PASS
    int current = initial;
    int next = 0;
    while (next > -1) {
        // pick random direction
        Direction dir = directions[distrib(gen)];
        next = walkOneStepFirstPass(current, dir);
        current = next;
    }

    // SECOND PASS
    // start at origin cell and trace directions
//    std::cout << "second pass" << std::endl;
    current = initial;
    next = 0;
    while (next > -1) {
        next = walkOneStepSecondPass(current);
        current = next;
    }
}

// Walk one step from location loc (as index) in direction Dir
// if step is valid, will visit that cell, otherwise will do nothing
// returns index of next cell or -1 if walking is done
// if direction is not valid, returns same index as start
int MazeBlock::walkOneStepFirstPass(int loc, Direction dir) {
    int next = -1;
    // external case: check borders
    if (hasExternalBorderInDirection(loc, dir)) {
        cells[loc].exitDir = dir;
    } else {
        // internal case
        next = getIndexOfCellNeighbor(loc, dir);
        if (next == -1 || cells[next].type == CellType::Closed) {
            return loc;
        }

        // if valid, mark direction of previous cell
        cells[loc].exitDir = dir;
        // repeat until visited cell is in mazeCells
        if (isCellInMaze(next)) {
            next = -1;
        }
    }
    return next;
}

int MazeBlock::walkOneStepSecondPass(int loc) {
    // add cells to mazeCells
    Cell &currentCell = cells[loc];
    currentCell.type = CellType::Open;
    mazeCells.insert(loc);
    int next = -1;

    // find exit direction and next cell
    Direction dir = currentCell.exitDir;

    // external case: check borders
    if (hasExternalBorderInDirection(loc, dir)) {
        // find correct border cell
        Cell &nextCell = getCellFromExternalBorder(loc, dir);
        // mark walls
        makePathBetweenCells(currentCell, nextCell, dir);
    } else {
        // internal case
        // get next cell and mark walls
        next = getIndexOfCellNeighbor(loc, dir);
        if (next == -1) { return next; }
        //        std::cout << "next: " << next << std::endl;
        Cell &nextCell = cells[next];
        makePathBetweenCells(currentCell, nextCell, dir);

        // continue until all cells in this walk visited
        if (isCellInMaze(next)) {
            next = -1;
        }
    }
    return next;
}

void MazeBlock::makePathBetweenCells(Cell &first, Cell &second, Direction dir) {
    switch(dir) {
    case Direction::N:
        first.northOpen = true;
        second.southOpen = true;
        break;
    case Direction::E:
        first.eastOpen = true;
        second.westOpen = true;
        break;
    case Direction::S:
        first.southOpen = true;
        second.northOpen = true;
        break;
    case Direction::W:
        first.westOpen = true;
        second.eastOpen = true;
        break;
    }
}

// randomly places closed spaces to be used for decor later
void MazeBlock::insertClosedSpaces() {
    static std::random_device rd; // a seed source for the random number engine
    static std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distribLoc(0, cells.size()-1);

    // add one closed space to a random location
    // 1x1 space for small mazes
    int loc;
    if (size() < 25) {
        loc = distribLoc(gen);
        cells[loc].type = CellType::Closed;
    //    std::cout << "closed: " << loc << std::endl;
        closedCellsCount += 1;
    } else {
        // 2x2 space for larger mazes
        // ensure it fits in the space
        while (true) {
            loc = distribLoc(gen);
            auto[x,y] = getCoordFromIndex(loc);
            if (x+1 > width-1 || y+1 > height-1) {
                continue;
            }
            cells[loc].type = CellType::Closed;
            cells[getIndexOfCellAt(x+1,y)].type = CellType::Closed;
            cells[getIndexOfCellAt(x,y+1)].type = CellType::Closed;
            cells[getIndexOfCellAt(x+1,y+1)].type = CellType::Closed;
            closedCellsCount += 4;
            break;
        }
    }
}

// returns the maze in string format
// "W" for wall, "O" for open space, "C" for closed space
std::string MazeBlock::toString(bool undensify, bool includeNewLines) {
    std::string mazeStr;
    if (undensify) {
        mazeStr = undensifyMaze(includeNewLines);
    } else {
        for (int i=0; i<size(); i++) {
            mazeStr += cells[i].toString();\
            if ((i+1)%width==0 && includeNewLines) {
               mazeStr += "\n";
            }
        }
    }
    return mazeStr;
}

// maze is initially generated in "dense" format where only walls don't take up space
// this function converts maze to string format where walls take up 1 block of space
// and will roughly double size of maze
std::string MazeBlock::undensifyMaze(bool includeNewLines) {
    std::string mazeStr;
//    // first row is all wall
//    for (int i=0; i<width*2+1; i++) {
//        mazeStr += "W";
//    }
//    // left wall
//    mazeStr += "\nW";

    for (int i=0; i<size(); i++) {
        mazeStr += cells[i].toString();

        // undensify horizontal space
        // ignore last column
        if ((i+1)%width!=0) {
            if (cells[i].eastOpen) {
                mazeStr += "O";
            } else if (cells[i].type == CellType::Closed && cells[i+1].type == CellType::Closed) {
                mazeStr += "C";
            } else {
                mazeStr += " ";
            }
        } else {
            // handle end of row by adding undensified row, i.e. undensify vertical space
//            // right wall
//            mazeStr += "W\n";
            if (includeNewLines) {
                mazeStr += "\n";
            }
            // ignore last row since row after that will be all wall
            if (i != size()-1) {
//                // left wall
//                mazeStr += "W";

                // look at row just iterated over
                // "i" is at end of row
                for (int j=i-width+1; j<i+1; j++) {
                    if (cells[j].southOpen) {
                        mazeStr += "O";
                    } else if ((cells[j].type == CellType::Closed && cells[j+width].type == CellType::Closed)) {
                        mazeStr += "C";
                    } else {
                        mazeStr += " ";
                    }
                    // account for horizontal undensification
                    if (j<i) {
                        if (cells[j].type == CellType::Closed && cells[j+1+width].type == CellType::Closed) {
                            mazeStr += "O";
                        } else {
                            mazeStr += " ";
                        }
                    }
                }
//                // right wall and left wall
                if (includeNewLines) {
                    mazeStr += "\n";
                }
//                mazeStr += "W\nW";
            }
        }
    }
//    // last row is all wall
//    mazeStr += std::string(width*2+1,'W');
    return mazeStr;
}

// returns references to the cells on the specified edge of the maze
// e.g. north is the top side, east is the right side
// ordered top to bottom, left to right
std::vector<std::reference_wrapper<Cell>> MazeBlock::getInternalBorderCells(Direction dir) {
    std::vector<std::reference_wrapper<Cell>> borderCells;
    switch(dir) {
    case Direction::N:
        for (int i=0; i<width; i++) {
            Cell& ref = cells[i];
            borderCells.emplace_back(ref);
        }
        break;
    case Direction::E:
        for (int i=0; i<height; i++) {
            Cell& ref = cells[(i+1)*width-1];
            borderCells.emplace_back(ref);
        }
        break;
    case Direction::S:
        for (int i=size()-width; i<size(); i++) {
            Cell& ref = cells[i];
            borderCells.emplace_back(ref);
        }
        break;
    case Direction::W:
        for (int i=0; i<height; i++) {
            Cell& ref = cells[i*width];
            borderCells.emplace_back(ref);
        }
        break;
    }
    return borderCells;
}

// sets the border cells that are external to the actual maze
void MazeBlock::setExternalBorderCells(std::vector<std::reference_wrapper<Cell>> refCells, Direction dir) {
    switch(dir) {
    case Direction::N:
        topExternalBorderCells = refCells;
        break;
    case Direction::E:
        rightExternalBorderCells = refCells;
        break;
    case Direction::S:
        bottomExternalBorderCells = refCells;
        break;
    case Direction::W:
        leftExternalBorderCells = refCells;
        break;
    }
}

bool MazeBlock::hasExternalBorderInDirection(int index, Direction dir) {
    if (!hasDefinedExternalBorderCells()) { return false; }
    auto[x,y] = getCoordFromIndex(index);
    switch(dir) {
    case Direction::N:
        return (y==0 && topExternalBorderCells.size()>0);
        break;
    case Direction::E:
        return (x==width-1 && rightExternalBorderCells.size()>0);
        break;
    case Direction::S:
        return (y==height-1 && bottomExternalBorderCells.size()>0);
        break;
    case Direction::W:
        return (x==0 && leftExternalBorderCells.size()>0);
        break;
    }
}

// assume external border exists in the correct direction and the index is correct
Cell& MazeBlock::getCellFromExternalBorder(int index, Direction dir) {
    auto[x,y] = getCoordFromIndex(index);
    switch(dir) {
    case Direction::N:
        return topExternalBorderCells[x];
        break;
    case Direction::E:
        return rightExternalBorderCells[y];
        break;
    case Direction::S:
        return bottomExternalBorderCells[x];
        break;
    case Direction::W:
        return leftExternalBorderCells[y];
        break;
    }
}
