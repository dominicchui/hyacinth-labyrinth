#include "maze.h"
#include <iostream>

// creates the 9 maze blocks
void Maze::generate() {
    // start with center
//    std::cout << "center" << std::endl;
    MazeBlock center = MazeBlock(width, height);
    center.generate();
    mazeBlocks[4] = center;

    // generate left, right, top, and bottom blocks
    std::cout << "left" << std::endl;
    MazeBlock left = MazeBlock(width, height);
    left.rightNeighbor = &center;
    center.leftNeighbor = &left;
//    left.setExternalBorderCells(center.getInternalBorderCells(Direction::W), Direction::E);
    left.generate();
    mazeBlocks[3] = left;

//    std::cout << "right" << std::endl;
    MazeBlock right = MazeBlock(width, height);
    right.leftNeighbor = &center;
    center.rightNeighbor = &right;
//    right.setExternalBorderCells(center.getInternalBorderCells(Direction::E), Direction::W);
    right.generate();
    mazeBlocks[5] = right;

//    std::cout << "top" << std::endl;
    MazeBlock top = MazeBlock(width, height);
    top.bottomNeighbor = &center;
    center.topNeighbor = &top;
//    top.setExternalBorderCells(center.getInternalBorderCells(Direction::N), Direction::S);
    top.generate();
    mazeBlocks[1] = top;

//    std::cout << "bottom" << std::endl;
    MazeBlock bottom = MazeBlock(width, height);
    bottom.topNeighbor = &center;
    center.bottomNeighbor = &bottom;
//    bottom.setExternalBorderCells(center.getInternalBorderCells(Direction::S), Direction::N);
    bottom.generate();
    mazeBlocks[7] = bottom;

    // generate top left, top right, bottom left, and bottom right blocks
//    std::cout << "top left" << std::endl;
    MazeBlock topLeft = MazeBlock(width, height);
    topLeft.rightNeighbor = &top;
    top.leftNeighbor = &topLeft;
    topLeft.bottomNeighbor = &left;
    left.topNeighbor = &topLeft;
//    topleft.setExternalBorderCells(top.getInternalBorderCells(Direction::W), Direction::E);
//    topleft.setExternalBorderCells(left.getInternalBorderCells(Direction::N), Direction::S);
    topLeft.generate();
    mazeBlocks[0] = topLeft;

//    std::cout << "top right" << std::endl;
    MazeBlock topRight = MazeBlock(width, height);
    topRight.leftNeighbor = &top;
    top.rightNeighbor = &topRight;
    topRight.bottomNeighbor = &right;
    right.topNeighbor = &topRight;
//    topRight.setExternalBorderCells(top.getInternalBorderCells(Direction::E), Direction::W);
//    topRight.setExternalBorderCells(right.getInternalBorderCells(Direction::N), Direction::S);
    topRight.generate();
    mazeBlocks[2] = topRight;

//    std::cout << "bottom left" << std::endl;
    MazeBlock bottomLeft = MazeBlock(width, height);
    bottomLeft.topNeighbor = &left;
    left.bottomNeighbor = &bottomLeft;
    bottomLeft.rightNeighbor = &bottom;
    bottom.leftNeighbor = &bottomLeft;
//    bottomLeft.setExternalBorderCells(left.getInternalBorderCells(Direction::S), Direction::N);
//    bottomLeft.setExternalBorderCells(bottom.getInternalBorderCells(Direction::W), Direction::E);
    bottomLeft.generate();
    mazeBlocks[6] = bottomLeft;

//    std::cout << "bottom right" << std::endl;
    MazeBlock bottomRight = MazeBlock(width, height);
    bottomRight.topNeighbor = &right;
    right.bottomNeighbor = &bottomRight;
    bottomRight.leftNeighbor = &bottom;
    bottom.rightNeighbor = &bottomRight;
//    bottomRight.setExternalBorderCells(right.getInternalBorderCells(Direction::S), Direction::N);
//    bottomRight.setExternalBorderCells(bottom.getInternalBorderCells(Direction::E), Direction::W);
    bottomRight.generate();
    mazeBlocks[8] = bottomRight;
}

// composes the string representation of the 9 constituent blocks
std::string Maze::toString() {
    std::string mazeStr = "";
    std::vector<std::string> mazeBlockStrs;

    for (int i=0; i<mazeBlocks.size(); i++) {
        mazeBlockStrs.emplace_back(mazeBlocks[i].toString());
//        mazeStr += mazeBlocks[i].toString() + "\n\n";
    }

    // temp testing
//    std::cout <<"top: " << std::endl;
//    std::cout << mazeBlocks[1].toString(true, true) << std::endl;
//    std::cout <<"center: " << std::endl;
//    std::cout << mazeBlocks[4].toString(true, true) << std::endl;
//    std::cout <<"right: " << std::endl;
//    std::cout << mazeBlocks[5].toString(true, true) << std::endl;


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


// composes string representaiton of three horizontal blocks together
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
            if (mazeBlocks[0].cells[last].eastOpen) {
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
            if (mazeBlocks[0].cells[last].eastOpen) {
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
        if (mazeBlocks[topBlockIndex].cells[index].southOpen) {
            mazeStr += "O";
        } else {
            mazeStr += " ";
        }
        mazeStr += " ";
    }
    // upper center block
    for (int i=0; i<width; i++) {
        index = i + width * (height-1);
        if (mazeBlocks[topBlockIndex+1].cells[index].southOpen) {
            mazeStr += "O";
        } else {
            mazeStr += " ";
        }
        mazeStr += " ";
    }
    // upper right block
    for (int i=0; i<width; i++) {
        index = i + width * (height-1);
        if (mazeBlocks[topBlockIndex+2].cells[index].southOpen) {
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
