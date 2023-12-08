#include "maze.h"
#include <iostream>
int main(int argc, char *argv[]) {
//    MazeBlock mazeBlock = MazeBlock(3,3);
//    mazeBlock.generate();
//    std::cout << mazeBlock.toString(true, true) << std::endl;


    Maze maze = Maze(3,3);
    maze.generate();
    std::cout << maze.toString() << std::endl;
}
