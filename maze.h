#pragma once

#include "cell.h"
#include <vector>
#include <set>
#include <random>


class Maze
{
public:
    Maze();
    Maze(int width, int height);

    int width;
    int height;
    std::vector<Cell> cells;
    std::set<int> mazeCells = std::set<int>();

//    static std::random_device rd;  // a seed source for the random number engine
//    static std::mt19937 gen; // mersenne_twister_engine

    int size() { return width * height; }
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
    bool isCellInMaze(int index) {
        return mazeCells.find(index) != mazeCells.end();
    }
    void makePathBetweenCells(Cell &first, Cell &second, Direction dir) {
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

    int getRandomEmptyCell();
    void generateMaze();
    void performRandomWalk();
    void visitCell(int index);

    // maze is generated in compact fashion, so add unit width walls when printing
    void printMaze(bool undensify = true);

private:
    std::string undensifyMaze();
};
