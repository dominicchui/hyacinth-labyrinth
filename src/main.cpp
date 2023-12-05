#include "hyacinth-labyrinth.hpp"

#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("Hyacinth Labyrinth");
    QCoreApplication::setOrganizationName("FEDZ");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    HyacinthLabyrinth app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
