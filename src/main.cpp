#include "hyacinth-labyrinth.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
    HyacinthLabyrinth app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
