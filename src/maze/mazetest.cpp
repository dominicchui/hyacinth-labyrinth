#include "maze.h"
#include <iostream>
int main(int argc, char *argv[]) {
    // temp maze testing
    Maze maze = Maze(3,3);
    maze.generate();
    std::cout << maze.toString() << std::endl;
}
