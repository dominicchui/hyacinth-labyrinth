#include "sceneparser.h"
#include "scenefilereader.h"
#include "timer.h"

#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>


void printNode(const SceneNode& node) {
    std::cout << "Node address 0x" << uint64_t(&node) << std::endl <<
        "\tnum children = " << node.children.size() << std::endl <<
        "\tnum lights = " << node.lights.size() << std::endl <<
        "\tnum primitives = " << node.primitives.size() << std::endl <<
        "\tnum transformations = " << node.transformations.size() << std::endl;
}

SceneLightData SLD_from_SL(SceneLight& sl, const glm::mat4& ctm) {
    return SceneLightData{
        sl.id,
        sl.type,
        sl.color,
        sl.function,
        ctm * glm::vec4{0.f,0.f,0.f,1.f},
        ctm * sl.dir,
        sl.penumbra,
        sl.angle,
        sl.width,
        sl.height
    };
}

void traverseScene(RenderData& renderData, SceneNode* node) {
    // Keep track of the parent's CTMs.
    // Start with the identity matrix at the root.
    std::list<glm::mat4> ctm_list;
    ctm_list.push_back(glm::mat4(1.0));

    glm::mat4& cur_ctm = ctm_list.back();

    // Depth first traversal
    // We do need to keep track of the CTM matrix at each node
    std::list<std::pair<SceneNode*, glm::mat4*>> to_visit;
    to_visit.push_back({node, &cur_ctm});

    while (!to_visit.empty()) {
        std::pair<SceneNode*, glm::mat4*> cur_pair = to_visit.back();
        to_visit.pop_back();

        SceneNode& cur_node = *cur_pair.first;
        glm::mat4 cur_ctm  = *cur_pair.second;

        //        printNode(cur_node);
        //        std::cout << "Current CTM";
        //        printMat4(cur_ctm);

        // First, check if we need to update our CTM.
        // We currently do not support multiple transformations of the same type
//        glm::mat4 trans(1.0);https://cs1230.graphics/projects/ray/1-algo/
//        glm::mat4 rot(1.0);
//        glm::mat4 scale(1.0);
//        glm::mat4 custom_mat(1.0);


        for (SceneTransformation* tform : cur_node.transformations) {
            switch(tform->type) {
            case TransformationType::TRANSFORMATION_TRANSLATE:
                cur_ctm = cur_ctm * glm::translate(tform->translate);
                break;
            case TransformationType::TRANSFORMATION_ROTATE:
                cur_ctm = cur_ctm * glm::rotate(tform->angle, tform->rotate);
                break;
            case TransformationType::TRANSFORMATION_SCALE:
                cur_ctm = cur_ctm * glm::scale(tform->scale);
                break;
            case TransformationType::TRANSFORMATION_MATRIX:
                cur_ctm = cur_ctm * tform->matrix;
                break;

            default:
                throw std::runtime_error(
                    "Got an invalid transformation type " + std::to_string(int32_t(tform->type))
                    );
            }
        }
        //cur_ctm = cur_ctm * custom_mat * trans * rot * scale;
        //        std::cout << "Pushing: ";
        //        printMat4(cur_ctm);
        ctm_list.push_back(cur_ctm);

        // Second, handle all the light data
        for (SceneLight* light : cur_node.lights) {
            renderData.lights.push_back(SLD_from_SL(*light, cur_ctm));

            //            std::cout << "Pushed light: " << light->id << std::endl;
        }

        // Third, handle all the primitives
        for (ScenePrimitive* prim : cur_node.primitives) {
            renderData.shapes.push_back({*prim, cur_ctm});
            //            std::cout << "Pushed light: " << int(prim->type) << std::endl;
        }

        // Lastly, see if there are any children.
        for (SceneNode* child : cur_node.children) {
            to_visit.push_back({child, &ctm_list.back()});
        }
    }
}

void traverseSceneRecursiveHelper(
    RenderData& renderData,
    SceneNode& node,
    glm::mat4& parent_ctm
    ) {
    //        printNode(cur_node);
    //        std::cout << "Current CTM";
    //        printMat4(cur_ctm);

    // First, check if we need to update our CTM.
    // We currently do not support multiple transformations of the same type
    glm::mat4 trans(1.0);
    glm::mat4 rot(1.0);
    glm::mat4 scale(1.0);
    glm::mat4 custom_mat(1.0);

    glm::mat4 cur_ctm = parent_ctm;

    for (SceneTransformation* tform : node.transformations) {
        switch(tform->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            cur_ctm = cur_ctm * glm::translate(tform->translate);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            cur_ctm = cur_ctm * glm::rotate(tform->angle, tform->rotate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            cur_ctm = cur_ctm * glm::scale(tform->scale);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            cur_ctm = cur_ctm * tform->matrix;
            break;

        default:
            throw std::runtime_error(
                "Got an invalid transformation type " + std::to_string(int32_t(tform->type))
            );
        }
    }

    //        printMat4(cur_ctm);

    // Second, handle all the light data
    for (SceneLight* light : node.lights) {
        renderData.lights.push_back(SLD_from_SL(*light, cur_ctm));

        //            std::cout << "Pushed light: " << light->id << std::endl;
    }

    // Third, handle all the primitives
    for (ScenePrimitive* prim : node.primitives) {
        renderData.shapes.push_back({*prim, cur_ctm});
        //            std::cout << "Pushed light: " << int(prim->type) << std::endl;
    }

    // Lastly, see if there are any children.
    for (SceneNode* child : node.children) {
        traverseSceneRecursiveHelper(renderData, *child, cur_ctm);
    }
}

void traverseSceneRecursive(RenderData& renderData, SceneNode* node) {
    glm::mat4 init_ctm(1.0);
    traverseSceneRecursiveHelper(renderData, *node, init_ctm);
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    mytimer t;
    //traverseScene(renderData, fileReader.getRootNode());

    // Turns out recursion sometimes is faster than goofy structures!
    traverseSceneRecursive(renderData, fileReader.getRootNode());
    t.checkpoint();

    std::cout << "Traversed the scene in " << t.to_string() << std::endl;

    return true;
}
