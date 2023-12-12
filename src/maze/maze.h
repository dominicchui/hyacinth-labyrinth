#pragma once
#include "mazeblock.h"

class Maze
{
public:
    Maze(){};
    Maze(int _width, int _height): denseBlockWidth(_width), denseBlockHeight(_height) {
        mazeBlocks.resize(9);
    };

    char WALL_REPRESENTATION = ' ';
    char PATH_REPRESENTATION = 'O';
    char CLOSED_AREA_REPRESENTATION = 'C';

    void generate();
    std::string toString();
    std::vector<std::vector<bool>> toBoolVector();

    void shiftLeft();
    void shiftRight();
    void shiftUp();
    void shiftDown();

    void addExtraPaths();

    int getBlockIndexFromPosition(float x, float y, float center_x, float center_y);

    // height and width of constituent maze blocks, not the maze itself
    // actual maze height ≈ 3*height
    int denseBlockWidth = 0;
    int denseBlockHeight = 0;
    // block width and height are affected by undensification
    int blockWidth = denseBlockWidth * 2 - 1;
    int blockHeight = denseBlockHeight * 2 - 1;
    // actual width and height of undensified maze
    int width = blockWidth*3+2;
    int height = blockHeight*3+2;

private:

    // the 3x3 grid of maze blocks that compose this maze
    std::vector<MazeBlock*> mazeBlocks;

    void generateMazeBlock(int index);
    void shift(Direction dir);

    std::string composeBlocks(std::vector<std::string> &mazeBlockStrs, int startingIndex);
    std::string getVerticalUndensificationString(int topBlockIndex);
    void addExtraPathBetweenBlocks(int first, int second);
};
