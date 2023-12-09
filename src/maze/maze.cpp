#include "maze.h"
#include <iostream>


void linkMazeBlocks(MazeBlock* first, MazeBlock* second, Direction dir) {
    switch(dir) {
    case Direction::N:
        first->topNeighbor = second;
        second->bottomNeighbor = first;
        break;
    case Direction::E:
        first->rightNeighbor = second;
        second->leftNeighbor = first;
        break;
    case Direction::S:
        first->bottomNeighbor = second;
        second->topNeighbor = first;
        break;
    case Direction::W:
        first->leftNeighbor = second;
        second->rightNeighbor = first;
        break;
    }
}

// creates the 9 maze blocks
void Maze::generate() {
    mazeBlocks.reserve(9);
    // start with center
    MazeBlock* center = new MazeBlock(width, height);
    center->generate();
    center->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[4] = center;

    // generate left, right, top, and bottom blocks
    MazeBlock* left = new MazeBlock(width, height);
    linkMazeBlocks(left, center, Direction::E);
    left->generate();
    left->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[3] = left;

    MazeBlock* right = new MazeBlock(width, height);
    linkMazeBlocks(right, center, Direction::W);
    right->generate();
    right->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[5] = right;

    MazeBlock* top = new MazeBlock(width, height);
    linkMazeBlocks(top, center, Direction::S);
    top->generate();
    top->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[1] = top;

    MazeBlock* bottom = new MazeBlock(width, height);
    linkMazeBlocks(bottom, center, Direction::N);
    bottom->generate();
    bottom->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[7] = bottom;

    // generate top left, top right, bottom left, and bottom right blocks
    MazeBlock* topLeft = new MazeBlock(width, height);
    linkMazeBlocks(topLeft, top, Direction::E);
    linkMazeBlocks(topLeft, left, Direction::S);
    topLeft->generate();
    topLeft->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[0] = topLeft;

    MazeBlock* topRight = new MazeBlock(width, height);
    linkMazeBlocks(topRight, top, Direction::W);
    linkMazeBlocks(topRight, right, Direction::S);
    topRight->generate();
    topRight->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[2] = topRight;

    MazeBlock* bottomLeft = new MazeBlock(width, height);
    linkMazeBlocks(bottomLeft, bottom, Direction::E);
    linkMazeBlocks(bottomLeft, left, Direction::N);
    bottomLeft->generate();
    bottomLeft->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[6] = bottomLeft;

    MazeBlock* bottomRight = new MazeBlock(width, height);
    linkMazeBlocks(bottomRight, bottom, Direction::W);
    linkMazeBlocks(bottomRight, right, Direction::N);
    bottomRight->generate();
    bottomRight->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
    mazeBlocks[8] = bottomRight;
}

// shifts the entire maze one block in the direction dir
// this will change the absolute positions of each maze block
// maze blocks not in the 3x3 centered on the new center will be deleted
// new maze blocks will be generated to replace them
void Maze::shift(Direction dir) {
    if (dir == Direction::N) {
        // delete blocks in first row
        delete mazeBlocks[0];
        delete mazeBlocks[1];
        delete mazeBlocks[2];

        // shift all blocks one up and drop first row
        for (int i=0; i<6; i++) {
            mazeBlocks[i] = mazeBlocks[i+3];
        }

        // generate 3 more blocks
        MazeBlock* bottom = new MazeBlock(width, height);
        linkMazeBlocks(bottom, mazeBlocks[4], Direction::N);
        bottom->generate();
        bottom->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[7] = bottom;

        MazeBlock* bottomLeft = new MazeBlock(width, height);
        linkMazeBlocks(bottomLeft, bottom, Direction::E);
        linkMazeBlocks(bottomLeft, mazeBlocks[3], Direction::N);
        bottomLeft->generate();
        bottomLeft->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[6] = bottomLeft;

        MazeBlock* bottomRight = new MazeBlock(width, height);
        linkMazeBlocks(bottomRight, bottom, Direction::W);
        linkMazeBlocks(bottomRight, mazeBlocks[5], Direction::N);
        bottomRight->generate();
        bottomRight->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[8] = bottomRight;
    } else if (dir == Direction::E) {
        // delete blocks on right side
        delete mazeBlocks[2];
        delete mazeBlocks[5];
        delete mazeBlocks[8];

        // shift all blocks one right
        for (int i=8; i>0; i--) {
            if (i!=3 && i!=6) {
                mazeBlocks[i] = mazeBlocks[i-1];
            }
        }

        // generate 3 more blocks
        MazeBlock* left = new MazeBlock(width, height);
        linkMazeBlocks(left, mazeBlocks[4], Direction::E);
        left->generate();
        left->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[3] = left;

        MazeBlock* topLeft = new MazeBlock(width, height);
        linkMazeBlocks(topLeft, mazeBlocks[1], Direction::E);
        linkMazeBlocks(topLeft, left, Direction::S);
        topLeft->generate();
        topLeft->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[0] = topLeft;

        MazeBlock* bottomLeft = new MazeBlock(width, height);
        linkMazeBlocks(bottomLeft, mazeBlocks[7], Direction::E);
        linkMazeBlocks(bottomLeft, left, Direction::N);
        bottomLeft->generate();
        bottomLeft->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[6] = bottomLeft;
    } else if (dir == Direction::S) {
        // delete blocks on bottom side
        delete mazeBlocks[6];
        delete mazeBlocks[7];
        delete mazeBlocks[8];

        // shift all blocks one down
        for (int i=8; i>2; i--) {
            mazeBlocks[i] = mazeBlocks[i-3];
        }

        // generate three more blocks
        MazeBlock* top = new MazeBlock(width, height);
        linkMazeBlocks(top, mazeBlocks[4], Direction::S);
        top->generate();
        top->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[1] = top;

        MazeBlock* topLeft = new MazeBlock(width, height);
        linkMazeBlocks(topLeft, mazeBlocks[1], Direction::E);
        linkMazeBlocks(topLeft, mazeBlocks[3], Direction::S);
        topLeft->generate();
        topLeft->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[0] = topLeft;

        MazeBlock* topRight = new MazeBlock(width, height);
        linkMazeBlocks(topRight, top, Direction::W);
        linkMazeBlocks(topRight, mazeBlocks[5], Direction::S);
        topRight->generate();
        topRight->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[2] = topRight;

    } else if (dir == Direction::W) {
        // delete blocks on left side
        delete mazeBlocks[0];
        delete mazeBlocks[3];
        delete mazeBlocks[6];

        // shift all blocks one left
        for (int i=0; i<8; i++) {
            if (i!=2 && i!=5) {
                mazeBlocks[i] = mazeBlocks[i+1];
            }
        }

        // generate three more blocks
        MazeBlock* right = new MazeBlock(width, height);
        linkMazeBlocks(right, mazeBlocks[4], Direction::W);
        right->generate();
        right->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[5] = right;

        MazeBlock* topRight = new MazeBlock(width, height);
        linkMazeBlocks(topRight, mazeBlocks[1], Direction::W);
        linkMazeBlocks(topRight, mazeBlocks[5], Direction::S);
        topRight->generate();
        topRight->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[2] = topRight;

        MazeBlock* bottomRight = new MazeBlock(width, height);
        linkMazeBlocks(bottomRight, mazeBlocks[7], Direction::W);
        linkMazeBlocks(bottomRight, mazeBlocks[5], Direction::N);
        bottomRight->generate();
        bottomRight->assignStringRepresentations(WALL_REPRESENTATION, PATH_REPRESENTATION, CLOSED_AREA_REPRESENTATION);
        mazeBlocks[8] = bottomRight;
    }
}

// composes the string representation of the 9 constituent blocks
std::string Maze::toString() {
    std::string mazeStr = "";
    std::vector<std::string> mazeBlockStrs;

    for (int i=0; i<mazeBlocks.size(); i++) {
        mazeBlockStrs.push_back(mazeBlocks[i]->toString());
    }

    // compose top three blocks
    mazeStr += composeBlocks(mazeBlockStrs, 0);

    // vertical undensification
    mazeStr += getVerticalUndensificationString(0);

    // compose middle three blocks
    mazeStr += composeBlocks(mazeBlockStrs, 3);

    // vertical undensification
    mazeStr += getVerticalUndensificationString(3);

    // compose bottom three blocks
    mazeStr += composeBlocks(mazeBlockStrs, 6);

    return mazeStr;
}


// composes string representation of three horizontal blocks together
std::string Maze::composeBlocks(std::vector<std::string> &mazeBlockStrs, int startingIndex) {
    // handle row by row
    std::string mazeStr = "";
    int cellRow = 0;
    for (int i=0; i<blockHeight; i++) {
        // grab row from first (left block)
        mazeStr += mazeBlockStrs[startingIndex].substr(i*blockWidth, blockWidth);
        int last = (cellRow+1)*width-1;
        int first = cellRow*width;
        // figure out horizontal undensification
        // if i is even, then cell row, if i is odd, then vertical undensification row
        if (i%2==0) {
            // index of last cell in the row
            // or part is "temp" workaround of modify by ref bug
            if (mazeBlocks[startingIndex]->cells[last].eastOpen ||
                mazeBlocks[startingIndex+1]->cells[first].westOpen) {
                mazeStr += PATH_REPRESENTATION;
            } else {
                mazeStr += WALL_REPRESENTATION;
            }
        } else {
            mazeStr += WALL_REPRESENTATION;
        }

        // grab row from middle block
        mazeStr += mazeBlockStrs[startingIndex+1].substr(i*blockWidth, blockWidth);
        // horizontal undensification
        if (i%2==0) {
            // index of last cell in the row
            if (mazeBlocks[startingIndex+1]->cells[last].eastOpen ||
                mazeBlocks[startingIndex+2]->cells[first].westOpen) {
                mazeStr += PATH_REPRESENTATION;
            } else {
                mazeStr += WALL_REPRESENTATION;
            }
            cellRow += 1;
        } else {
            mazeStr += WALL_REPRESENTATION;
        }
        // grab row from right block
        mazeStr += mazeBlockStrs[startingIndex+2].substr(i*blockWidth, blockWidth);
        mazeStr += "\n";
    }
    return mazeStr;
}

std::string Maze::getVerticalUndensificationString(int topBlockIndex) {
    std::string mazeStr = "";
    // iterate through the three top blocks
    int index;
    // upper left block
    for (int i=0; i<width; i++) {
        index = i + width * (height-1);
        if (mazeBlocks[topBlockIndex]->cells[index].southOpen) {
            mazeStr += PATH_REPRESENTATION;
        } else {
            mazeStr += WALL_REPRESENTATION;
        }
        mazeStr += WALL_REPRESENTATION;
    }
    // upper center block
    for (int i=0; i<width; i++) {
        index = i + width * (height-1);
        if (mazeBlocks[topBlockIndex+1]->cells[index].southOpen) {
            mazeStr += PATH_REPRESENTATION;
        } else {
            mazeStr += WALL_REPRESENTATION;
        }
        mazeStr += WALL_REPRESENTATION;
    }
    // upper right block
    for (int i=0; i<width; i++) {
        index = i + width * (height-1);
        if (mazeBlocks[topBlockIndex+2]->cells[index].southOpen) {
            mazeStr += PATH_REPRESENTATION;
        } else {
            mazeStr += WALL_REPRESENTATION;
        }
        // ignore last cell
        if (i!=width-1) {
            mazeStr += WALL_REPRESENTATION;
        }
    }
    mazeStr += "\n";

    return mazeStr;
}
