#include "maze.h"
#include <iostream>


Maze::Maze()
{

}

Maze::Maze(int _width, int _height): width(_width), height(_height) {
    cells = std::vector(size(),Cell(CellType::Empty));
    std::cout << "size: " << size() << std::endl;
//    static std::random_device rd;
//    static std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
}

int Maze::getRandomEmptyCell() {
    if (mazeCells.size() == size()) { return -1; }
    // todo move for performance
    static std::random_device rd;
    static std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, size()-1);
    // todo maybe don't rely on luck
    while (true) {
        int n = distrib(gen);
        // check cell is valid target
        if (!isCellInMaze(n) && cells[n].type==CellType::Empty) {
            return n;
        }
    }
}

void Maze::generateMaze() {
    // add random cell to maze
    int n = getRandomEmptyCell();
    cells[n].type = CellType::Open;
    mazeCells.insert(n);
    std::cout << "first cell: " << n << std::endl;

    while (mazeCells.size() < size()) {
        performRandomWalk();
    }
}

void Maze::performRandomWalk() {
    std::cout << "walking..." << std::endl;
    // pick random empty cell
    int initial = getRandomEmptyCell();
    std::cout << "initial: " << initial << std::endl;

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
        std::cout << "next: " << next << std::endl;
        if (next == -1 || cells[next].type == CellType::Closed) {
            continue;
        }

        // if valid, mark direction of previous cell
        cells[current].exitDir = dir;
        // repeat until visited cell is in mazeCells
        if (isCellInMaze(next)) {
            break;
        }
        current = next;
    }

    std::cout << "second pass..." << std::endl;

    // SECOND PASS
    // start at origin cell and trace directions
    current = initial;
    while (true) {
        // add cells to mazeCells
        Cell &currentCell = cells[current];
        currentCell.type = CellType::Open;
        mazeCells.insert(current);

        // find next cell and mark walls
        int next = getIndexOfCellNeighbor(current, currentCell.exitDir);
        Cell &nextCell = cells[next];
        makePathBetweenCells(currentCell, nextCell, currentCell.exitDir);

        // continue until all cells in this walk visited
        if (isCellInMaze(next)) {
            break;
        }
        current = next;
    }
}

std::string Maze::undensifyMaze() {
    std::string mazeStr;
    // first row is all wall
    for (int i=0; i<width*2+1; i++) {
        mazeStr += "W";
    }
    // left wall
    mazeStr += "\nW";

    for (int i=0; i<size(); i++) {
        if (cells[i].type == CellType::Open) {
            mazeStr += "O";
        } else {
            mazeStr += "W";
        }

        // undensify horizontal space
        // ignore last column
        if ((i+1)%width!=0) {
            if (cells[i].eastOpen) {
                mazeStr += "O";
            } else {
                mazeStr += "W";
            }
        } else {
            // handle end of row
            // right wall
            mazeStr += "W\n";
            // ignore last row
            if (i != size()-1) {
                // undensify vertical space
                // left wall
                mazeStr += "W";

                // look at previous row
                for (int j=i-width+1; j<i+1; j++) {
                    if (cells[j].southOpen) {
                        mazeStr += "O";
                    } else {
                        mazeStr += "W";
                    }
                    // account for horizontal undensification
                    if (j<i) {
                        mazeStr += "W";
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

// maze is generated in "dense" format where only walls don't take up space
// convert to format where walls take up 1 block of space
// also enclose entirety in wall
// will roughly double size of maze
void Maze::printMaze(bool undensify) {
    if (undensify) {
        std::string mazeStr = undensifyMaze();
        std::cout << mazeStr << std::endl;
    } else {
        for (int i=0; i<size(); i++) {
            if (cells[i].type == CellType::Open) {
                std::cout << "O";
            } else {
                std::cout << "W";
            }
            // end of row
            if ((i+1)%width==0) {
                std::cout << "\n";
            }
        }
    }
}

/*
 * WWWOW
 * WOOOW
 * WOWWW
 * WOOOW
 * WWWOW
 */
