#pragma once

#include <GL/glew.h>
#include <iostream>
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "renderer/camera.h"
#include "scene/sceneparser.h"
#include "scene/objloader.h"

// Abstract class representing a primitive shape that can be rendered
class Shape {
private:
    glm::mat3 invTransCTM;
    SceneMaterial material;
    // updates the vertexData field based on the current parameters in the global settings object
    virtual void setVertexData();
    int startIdx; // for drawing
    int numVertex;

protected:
    glm::mat4 ctm;
    glm::mat4 inverseCTM;


    std::vector<GLfloat> vertexData;
    int param1;
    int param2;
    float radius = 0.5;
    // inserts a given vector into vertexData
    void insertVec3(glm::vec3 &v);

    void setCTM(glm::mat4 newCTM) {
        ctm = newCTM;
        inverseCTM = inverse(ctm);
        invTransCTM = transpose(inverse(glm::mat3(ctm)));
    }

public:
    // Constructor for an abstract shape. Should only be used by sub-classes
    Shape(RenderShapeData &data);
    // Generates the vertex data for this shape and adds that information to allVertex.
    // Returns the number of vertices that were added for this shape
    int updateData(int startIndex, std::vector<GLfloat> &allVertex);
    // Draws this shape in the scene.
    // IMPORTANT: assumes that the shader and VAO have already been bound.
    void draw(GLuint shader);
};

class Cone : public Shape {
private:
    void addSideNorm(glm::vec3 &point);
    void addBaseNorm(glm::vec3 &point);
    void makeSideTile(glm::vec3 &topLeft, glm::vec3 &topRight,
                      glm::vec3 &bottomLeft, glm::vec3 &bottomRight);
    void makeBaseTile(glm::vec3 &topLeft, glm::vec3 &topRight,
                      glm::vec3 &bottomLeft, glm::vec3 &bottomRight);
    void makeTip(glm::vec3 &left, glm::vec3 &right);
    void makeBaseCenter(glm::vec3 &left, glm::vec3 &right);
    void makeBase(glm::vec3 &left, glm::vec3 &right);
    void makeSide(glm::vec3 &left, glm::vec3 &right);
    void makeWedge(float startTheta, float endTheta);
    void setVertexData() override;
public:
    // Constructor for a primitive cone
    Cone(RenderShapeData &data) : Shape(data) {}
};

class Cube : public Shape {
private:
    void addTriNorm(glm::vec3 point, glm::vec3 norm);
    void makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                  glm::vec3 bottomLeft, glm::vec3 bottomRight);
    void makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                  glm::vec3 bottomLeft, glm::vec3 bottomRight);
    void setVertexData() override;
public:
    // Constructor for a primitive cube
    Cube(RenderShapeData &data) : Shape(data) {}

    inline float sqr(float v) { return v * v; }
    bool intersects(glm::vec3 center, float r) {
        glm::vec3 Bmin = ctm * glm::vec4(-radius, -radius, -radius, 1);
        glm::vec3 Bmax = ctm * glm::vec4(radius, radius, radius, 1);
        float r2 = r * r;
        float dmin = 0;
        for( int i = 0; i < 3; i++ ) {
            if (center[i] < Bmin[i] ) dmin += sqr(center[i] - Bmin[i] );
            else if (center[i] > Bmax[i] ) dmin += sqr(center[i] - Bmax[i] );
        }
        return dmin <= r2;
    }
};

class Cylinder : public Shape {
private:
    void addTriNorm(glm::vec3 &point, glm::vec3 &mask);
    void makeTile(glm::vec3 &topLeft, glm::vec3 &topRight,
                  glm::vec3 &bottomLeft, glm::vec3 &bottomRight,
                  glm::vec3 &normmask);
    void makeFace(glm::vec3 &topLeft, glm::vec3 &topRight,
                  glm::vec3 &bottomLeft, glm::vec3 &bottomRight);
    void makeFan(float height, glm::vec3 &left, glm::vec3 &right);
    void makeCap(float height, glm::vec3 &left, glm::vec3 &right);
    void makeWedge(float startTheta, float endTheta);
    void setVertexData() override;
public:
    // Constructor for a primitive cylinder
    Cylinder(RenderShapeData &data) : Shape(data) {}
};

class Sphere : public Shape {
private:
    glm::vec3 getCoord(float r, float theta, float phi);
    void addTriNorm(glm::vec3 &point);
    void makeTile(glm::vec3 &topLeft, glm::vec3 &topRight,
                  glm::vec3 &bottomLeft, glm::vec3 &bottomRight);
    void makeWedge(float startTheta, float endTheta);
    void setVertexData() override;

    glm::mat4 R = glm::mat4(1), T = glm::mat4(1);
    float ground =0;
    std::vector<Cube *> walls;

public:
    // Constructor for a primitive sphere
    Sphere(RenderShapeData &data) : Shape(data) {
        radius = 0.4;
    }

    void setBall(float gnd, std::vector<Cube *> wls) {
        ground = gnd;
        walls = wls;
    }
    bool onFloor() {
        return (ctm * glm::vec4(0, 0, 0, 1)).y <= ground;
    }
    bool noIntersections(glm::mat4 trans) {
        glm::vec3 center = trans * glm::vec4(0,0,0,1);
        for (Cube *wall : walls) {
            if (wall->intersects(center, radius)) return false;
        }
        return true;
    }
    void move(float dist, glm::vec3 dir) {
//        glm::vec3 up = normalize(inverseCTM * glm::vec4(0,1,0,0));
//        glm::vec3 rot = normalize(cross(dir,up));
//        R = glm::rotate(R, float(M_PI * dist / 4.f), rot);

        glm::vec3 move = dist * dir;

        glm::vec3 moveX = {move.x, 0, 0};
        glm::mat4 delta = glm::translate(T, moveX);
        if (noIntersections(delta)) T = delta;

        moveX = {0, move.y, 0};
        delta = glm::translate(T, moveX);
        if (noIntersections(delta)) T = delta;

        moveX = {0, 0, move.z};
        delta = glm::translate(T, moveX);
        if (noIntersections(delta)) T = delta;

        setCTM(T * R);
    }
};

class Mesh : public Shape {
private:
    // Once the data has been loaded once, this field is set to true
    bool loaded = false;
    std::string filepath;
    ObjLoader loader;
    void setVertexData() override;
public:
    // Constructor for a mesh object
    Mesh(RenderShapeData &data) : Shape(data) {
        filepath = data.primitive.meshfile;
    }
};

// Given RenderShapeData, generates a shape of the appropriate type.
// This function uses "new", so the caller must delete and shapes
// generated by this function.
static Shape *getShape(RenderShapeData &data) {
    switch (data.primitive.type) {
    case PrimitiveType::PRIMITIVE_CUBE:  return new Cube(data);
    case PrimitiveType::PRIMITIVE_CONE: return new Cone(data);
    case PrimitiveType::PRIMITIVE_CYLINDER: return new Cylinder(data);
    case PrimitiveType::PRIMITIVE_SPHERE: return new Sphere(data);
    case PrimitiveType::PRIMITIVE_MESH: return new Mesh(data);
    }
    return nullptr;
}
