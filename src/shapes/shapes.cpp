#include "shapes.h"

#include "utils/settings.h"

Shape::Shape(RenderShapeData &data) {
    ctm = data.ctm;
    inverseCTM = inverse(ctm);
    invTransCTM = transpose(inverse(glm::mat3(ctm)));
    material = data.primitive.material;
}

void Shape::setVertexData() {
    std::cerr << "WARN: tried to set vertex data of abstract shape" << std::endl;
}

void Shape::insertVec3(glm::vec3 &v) {
    vertexData.push_back(v.x);
    vertexData.push_back(v.y);
    vertexData.push_back(v.z);
}

int Shape::updateData(int startIndex, std::vector<GLfloat> &allVertex) {
    startIdx = startIndex;
    vertexData.clear();
    param1 = settings.shapeParameter1;
    param2 = settings.shapeParameter2;
    setVertexData();

    // size is data/6, since each vertex has point (3) and normal (3)
    numVertex = vertexData.size() / 6;
    allVertex.insert(allVertex.end(), vertexData.begin(), vertexData.end());
    return numVertex;
}

void Shape::draw(GLuint shader) {
    // Set model matrix
    glUniformMatrix4fv(
        glGetUniformLocation(shader, "model"),
        1, GL_FALSE, &ctm[0][0]);

    // Set inverse transpose of model matrix
    glUniformMatrix3fv(
        glGetUniformLocation(shader, "invTransModel"),
        1, GL_FALSE, &invTransCTM[0][0]);

    // Ambient data
    glUniform4f(glGetUniformLocation(shader, "cAmb"),
                material.cAmbient[0], material.cAmbient[1],
                material.cAmbient[2], material.cAmbient[3]);

    // Diffuse data
    glUniform4f(glGetUniformLocation(shader, "cDiff"),
                material.cDiffuse[0], material.cDiffuse[1],
                material.cDiffuse[2], material.cDiffuse[3]);

    // Specular data
    glUniform1f(glGetUniformLocation(shader, "shininess"), material.shininess);
    glUniform4f(glGetUniformLocation(shader, "cSpec"),
                material.cSpecular[0], material.cSpecular[1],
                material.cSpecular[2], material.cSpecular[3]);

    glDrawArrays(GL_TRIANGLES, startIdx, numVertex);
}
