#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

void applyTrans(glm::mat4 &ctm, std::vector<SceneTransformation *> &transformations, bool a = false) {
    for (SceneTransformation *trans : transformations) {
        switch (trans->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm *= glm::translate(trans->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm *= glm::scale(trans->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            ctm *= glm::rotate(trans->angle, trans->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm *= trans->matrix;
            break;
        }
    }
}

void dfsScene(SceneNode* node, glm::mat4 ctm, std::vector<RenderShapeData> &shapes, std::vector<SceneLightData> &lights) {
    applyTrans(ctm, node->transformations);

    for (ScenePrimitive *shape : node->primitives) {
        shapes.push_back(RenderShapeData(*shape, ctm));
    }
    for (SceneLight *light : node->lights) {
        lights.push_back(SceneLightData(
            light->id,
            light->type,
            light->color,
            light->function,
            ctm * glm::vec4(0,0,0,1), // Position with CTM applied (Not applicable to directional lights)
            ctm * light->dir,         // Direction with CTM applied (Not applicable to point lights)
            light->penumbra,
            light->angle,
            light->width,
            light->height
            ));
    }

    for (SceneNode *child : node->children) {
        dfsScene(child, ctm, shapes, lights);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // Populate renderData with global data, and camera data;
    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();

    // Populate renderData's list of primitives and their transforms.
    renderData.shapes.clear();
    renderData.lights.clear();
    SceneNode *root = fileReader.getRootNode();

    dfsScene(root, glm::mat4(1.f), renderData.shapes, renderData.lights);

    return true;
}
