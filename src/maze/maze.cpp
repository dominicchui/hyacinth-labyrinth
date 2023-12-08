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
    mazeBlocks[4] = center;

    // generate left, right, top, and bottom blocks
    std::cout << "left" << std::endl;
    MazeBlock* left = new MazeBlock(width, height);
    linkMazeBlocks(left, center, Direction::E);
    left->generate();
    mazeBlocks[3] = left;

    MazeBlock* right = new MazeBlock(width, height);
    linkMazeBlocks(right, center, Direction::W);
    right->generate();
    mazeBlocks[5] = right;

    MazeBlock* top = new MazeBlock(width, height);
    linkMazeBlocks(top, center, Direction::S);
    top->generate();
    mazeBlocks[1] = top;

    MazeBlock* bottom = new MazeBlock(width, height);
    linkMazeBlocks(bottom, center, Direction::N);
    bottom->generate();
    mazeBlocks[7] = bottom;

    // generate top left, top right, bottom left, and bottom right blocks
    MazeBlock* topLeft = new  MazeBlock(width, height);
    linkMazeBlocks(topLeft, top, Direction::E);
    linkMazeBlocks(topLeft, left, Direction::S);
    topLeft->generate();
    mazeBlocks[0] = topLeft;

    MazeBlock* topRight = new MazeBlock(width, height);
    linkMazeBlocks(topRight, top, Direction::W);
    linkMazeBlocks(topRight, right, Direction::S);
    topRight->generate();
    mazeBlocks[2] = topRight;

    MazeBlock* bottomLeft = new MazeBlock(width, height);
    linkMazeBlocks(bottomLeft, bottom, Direction::E);
    linkMazeBlocks(bottomLeft, left, Direction::N);
    bottomLeft->generate();
    mazeBlocks[6] = bottomLeft;

    MazeBlock* bottomRight = new MazeBlock(width, height);
    linkMazeBlocks(bottomRight, bottom, Direction::W);
    linkMazeBlocks(bottomRight, right, Direction::N);
    bottomRight->generate();
    mazeBlocks[8] = bottomRight;
}

// composes the string representation of the 9 constituent blocks
std::string Maze::toString() {
    std::string mazeStr = "";
    std::vector<std::string> mazeBlockStrs;

    for (int i=0; i<mazeBlocks.size(); i++) {
        mazeBlockStrs.push_back(mazeBlocks[i]->toString());
//        mazeStr += mazeBlocks[i].toString() + "\n\n";
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
        // grab row from top left
        mazeStr += mazeBlockStrs[startingIndex].substr(i*blockWidth, blockWidth);
        int last = (cellRow+1)*width-1;
        // figure out horizontal undensification
        // if i is even, then cell row, if i is odd, then vertical undensification row
        if (i%2==0) {
            // index of last cell in the row
            if (mazeBlocks[0]->cells[last].eastOpen) {
                mazeStr += "O";
            } else {
                mazeStr += " ";
            }
        } else {
            mazeStr += " ";
        }

        // grab row from top
        mazeStr += mazeBlockStrs[startingIndex+1].substr(i*blockWidth, blockWidth);
        // horizontal undensification
        if (i%2==0) {
            // index of last cell in the row
            if (mazeBlocks[0]->cells[last].eastOpen) {
                mazeStr += "O";
            } else {
                mazeStr += " ";
            }
            cellRow += 1;
        } else {
            mazeStr += " ";
        }
        // grab row from top right
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
            mazeStr += "O";
        } else {
            mazeStr += " ";
        }
        mazeStr += " ";
    }
    // upper center block
    for (int i=0; i<width; i++) {
        index = i + width * (height-1);
        if (mazeBlocks[topBlockIndex+1]->cells[index].southOpen) {
            mazeStr += "O";
        } else {
            mazeStr += " ";
        }
        mazeStr += " ";
    }
    // upper right block
    for (int i=0; i<width; i++) {
        index = i + width * (height-1);
        if (mazeBlocks[topBlockIndex+2]->cells[index].southOpen) {
            mazeStr += "O";
        } else {
            mazeStr += " ";
        }
        // ignore last cell
        if (i!=width-1) {
            mazeStr += " ";
        }
    }
    mazeStr += "\n";

    return mazeStr;
}
