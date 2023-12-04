#include "maze.h"
#include <iostream>


Maze::Maze()
{

}

Maze::Maze(int _width, int _height): width(_width), height(_height) {
    cells = std::vector(size(),Cell(CellType::Empty));
}

Maze::Maze(int _width, int _height, bool _insertClosedSpaces): width(_width), height(_height) {
    cells = std::vector(size(),Cell(CellType::Empty));
    if (_insertClosedSpaces) {
        insertClosedSpaces();
    }
}

int Maze::getRandomEmptyCell() {
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
void Maze::generateMaze() {
    // add random cell to maze if no borders are defined
    if (!hasDefinedBorderCells()) {
        int n = getRandomEmptyCell();
        std::cout << "first: " << n << std::endl;
        cells[n].type = CellType::Open;
        mazeCells.insert(n);
    }

    while (mazeCells.size() + closedCellsCount < size()) {
        performRandomWalk();
    }
}

void Maze::performRandomWalk() {
    // pick random empty cell
    int initial = getRandomEmptyCell();
    std::cout << "walk start: " << initial << std::endl;

    // FIRST PASS
    static std::random_device rd; // a seed source for the random number engine
    static std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, directions.size());

    int current = initial;
    while (true) {
        // pick random direction
        Direction dir = directions[distrib(gen)];
        // get new cell and check validity
        int next = getIndexOfCellNeighbor(current, dir);
        if (next == -1 || cells[next].type == CellType::Closed) {
            continue;
        }
        std::cout << "next: " << next << std::endl;

        // if valid, mark direction of previous cell
        cells[current].exitDir = dir;
        // repeat until visited cell is in mazeCells
        if (isCellInMaze(next)) {
            break;
        }
        current = next;
    }

    // SECOND PASS
    // start at origin cell and trace directions
    std::cout << "second pass" << std::endl;
    current = initial;
    while (true) {
        // add cells to mazeCells
        Cell &currentCell = cells[current];
        currentCell.type = CellType::Open;
        mazeCells.insert(current);

        // find next cell and mark walls
        int next = getIndexOfCellNeighbor(current, currentCell.exitDir);
        std::cout << "next: " << next << std::endl;
        Cell &nextCell = cells[next];
        makePathBetweenCells(currentCell, nextCell, currentCell.exitDir);

        // continue until all cells in this walk visited
        if (isCellInMaze(next)) {
            break;
        }
        current = next;
    }
}

// randomly places closed spaces to be used for decor later
void Maze::insertClosedSpaces() {
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
std::string Maze::toString(bool undensify) {
    std::string mazeStr;
    if (undensify) {
        mazeStr = undensifyMaze();
    } else {
        for (int i=0; i<size(); i++) {
            mazeStr += cells[i].toString();\
            if ((i+1)%width==0) {
               mazeStr += "\n";
            }
        }
    }
    return mazeStr;
}

// maze is initially generated in "dense" format where only walls don't take up space
// this function converts maze to string format where walls take up 1 block of space
// will also enclose entirety in wall
// and will roughly double size of maze
std::string Maze::undensifyMaze() {
    std::string mazeStr;
    // first row is all wall
    for (int i=0; i<width*2+1; i++) {
        mazeStr += "W";
    }
    // left wall
    mazeStr += "\nW";

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
                mazeStr += "W";
            }
        } else {
            // handle end of row by adding undensified row, i.e. undensify vertical space
            // right wall
            mazeStr += "W\n";
            // ignore last row since row after that will be all wall
            if (i != size()-1) {
                // left wall
                mazeStr += "W";

                // look at row just iterated over
                // "i" is at end of row
                for (int j=i-width+1; j<i+1; j++) {
                    if (cells[j].southOpen) {
                        mazeStr += "O";
                    } else if ((cells[j].type == CellType::Closed && cells[j+width].type == CellType::Closed)) {
                        mazeStr += "C";
                    } else {
                        mazeStr += "W";
                    }
                    // account for horizontal undensification
                    if (j<i) {
                        if (cells[j].type == CellType::Closed && cells[j+1+width].type == CellType::Closed) {
                            mazeStr += "C";
                        } else {
                            mazeStr += "W";
                        }
                    }
                }
                // right wall and left wall
                mazeStr += "W\nW";
            }
        }
    }
    // last row is all wall
    mazeStr += std::string(width*2+1,'W');
    return mazeStr;
}

// returns references to the cells on the specified edge of the maze
// e.g. north is the top side, east is the right side
// ordered top to bottom, left to right
std::vector<std::reference_wrapper<Cell>> Maze::getBorderCells(Direction dir) {
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
