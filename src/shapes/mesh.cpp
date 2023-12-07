#include "shapes.h"

void Mesh::setVertexData() {
    if (!loaded) {
        loader.load(filepath);
        vertexData.clear();
    }

    for (glm::vec3 &data : loader.getData()) insertVec3(data);
    loaded = true;
}
