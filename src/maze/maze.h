#pragma once
#include "mazeblock.h"

class Maze
{
public:
    Maze(int _width, int _height): width(_width), height(_height) {
        mazeBlocks.resize(9);
    };

    char WALL_REPRESENTATION = ' ';
    char PATH_REPRESENTATION = 'O';
    char CLOSED_AREA_REPRESENTATION = 'C';

    void generate();
    std::string toString();
    void shift(Direction dir);

private:
    // height and width of constituent maze blocks, not the maze itself
    // actual maze height ≈ 3*height
    int width;
    int height;
    // block width and height are affected by undensification
    int blockWidth = width * 2 - 1;
    int blockHeight = height * 2 - 1;
    // the 3x3 grid of maze blocks that compose this maze
    std::vector<MazeBlock*> mazeBlocks;

    void generateMazeBlock(int index);

    std::string composeBlocks(std::vector<std::string> &mazeBlockStrs, int startingIndex);
    std::string getVerticalUndensificationString(int topBlockIndex);
};
