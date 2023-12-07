#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string sceneFilePath;
    int shapeParameter1 = 1;
    int shapeParameter2 = 1;
    float nearPlane = 1;
    float farPlane = 1;
    bool perPixelFilter = false; // invert colors
    bool kernelBasedFilter = false; // box blur
    bool extraCredit1 = false; // grayscale
    bool extraCredit2 = false; // sharpen
    bool extraCredit3 = false; // gamma tone mapping
    float ecGamma = 0; // gamma (for tone mapping)
    bool extraCredit4 = false; // laplacian edge detection
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

#endif // SETTINGS_H
