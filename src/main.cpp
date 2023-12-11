#define VULKAN_PROJ 1

#if VULKAN_PROJ == 1
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

#elif VULKAN_PROJ == 2

#include "ball-test.hpp"

#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("Hyacinth Labyrinth");
    QCoreApplication::setOrganizationName("FEDZ");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    BallTest app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#else
#include "window/mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <iostream>
#include <QSettings>
#include "maze/maze.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("Projects 5 & 6: Lights, Camera & Action!");
    QCoreApplication::setOrganizationName("CS 2230");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QSurfaceFormat fmt;
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow w;
    w.initialize();
    w.resize(800, 600);
    w.show();

    int return_val = a.exec();
    w.finish();
    return return_val;
}
#endif
