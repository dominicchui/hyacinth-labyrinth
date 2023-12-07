#pragma once

#include "cell.h"
#include <vector>
#include <set>
#include <random>

class MazeBlock
{
public:
    MazeBlock();
    MazeBlock(int width, int height);
    MazeBlock(int width, int height, bool insertClosedSpaces);

    int width;
    int height;
    std::vector<Cell> cells;

    int size() { return width * height; }

    std::vector<std::reference_wrapper<Cell>> getInternalBorderCells(Direction dir);
    void setExternalBorderCells(std::vector<std::reference_wrapper<Cell>>, Direction Dir);

    void generate();

    // maze is generated in compact fashion, so add unit width walls when converting to string
    std::string toString(bool undensify = true, bool includeNewLines = false);

private:
    std::set<int> mazeCells = std::set<int>();
    int closedCellsCount = 0;

    //    static std::random_device rd;  // a seed source for the random number engine
    //    static std::mt19937 gen; // mersenne_twister_engine

    int getIndexOfCellAt(int x, int y) {
        // ensure cell is valid
        if (x<0 || x>=width || y<0 || y>=width) {
            return -1;
        }
        return x + width * y;
    }
    std::tuple<int, int> getCoordFromIndex(int index) {
        int y = index / width;
        int x = index % width;
        return std::tuple<int, int> {x,y};
    }
    int getIndexOfCellNeighbor(int index, Direction dir) {
        auto[x,y] = getCoordFromIndex(index);
        int newIndex;
        switch(dir) {
        case Direction::N:
            newIndex = getIndexOfCellAt(x,y-1);
            break;
        case Direction::E:
            newIndex = getIndexOfCellAt(x+1,y);
            break;
        case Direction::S:
            newIndex = getIndexOfCellAt(x,y+1);
            break;
        case Direction::W:
            newIndex = getIndexOfCellAt(x-1,y);
            break;
        }
        return newIndex;
    }

    // 4 sets of cells that define the boundaries of this "Maze"
    // used to generate a maze with predefined borders
    // e.g. this maze is being generated to the right of an existing one and will be appended
    // order of cells is left to right, up to down
    // assumes the size will be width or height (as appropriate)
    std::vector<std::reference_wrapper<Cell>> leftExternalBorderCells;
    std::vector<std::reference_wrapper<Cell>> rightExternalBorderCells;
    std::vector<std::reference_wrapper<Cell>> topExternalBorderCells;
    std::vector<std::reference_wrapper<Cell>> bottomExternalBorderCells;

    bool hasExternalBorderInDirection(int index, Direction dir);
    Cell& getCellFromExternalBorder(int index, Direction dir);
    bool isCellInMaze(int index) {
        return mazeCells.find(index) != mazeCells.end();
    }
    void makePathBetweenCells(Cell &first, Cell &second, Direction dir);

    bool hasDefinedExternalBorderCells() {
        return (leftExternalBorderCells.size()!=0 || rightExternalBorderCells.size()!=0 ||
                topExternalBorderCells.size()!=0 || bottomExternalBorderCells.size()!=0);
    }
    /* internal border cells belong to this maze, external ones do not and instead surround the maze
    e.g. I=internal, E=External, M=other maze cells for this 3x3 maze
    EEEEE
    EIIIE
    EIMIE
    EIIIE
    EEEEE
    */
    int getRandomEmptyCell();
    void performRandomWalk();
    int walkOneStepFirstPass(int loc, Direction dir);
    int walkOneStepSecondPass(int loc);
    void insertClosedSpaces();
    std::string undensifyMaze(bool includeNewLines);
};
