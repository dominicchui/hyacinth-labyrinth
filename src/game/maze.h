#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <random>

#include "vulkan/vulkan-model.hpp"
#include "vulkan/vulkan-device.hpp"
#include "utils/utils.h"
#include "lve_game_object.hpp"

// struct pair_hash {
//     template <class T1, class T2>
//     std::size_t operator() (std::pair<T1, T2> const &v) const
//     {
//         int64_t v1 = int64_t(v.first);
//         int64_t v2 = int64_t(v.second) + (1LL << 32LL);
//         int64_t val = std::hash<int64_t>()(v1 + v2);
//         return val;
//     }
// };

class GameMaze {
private:
    bool maze_valid;
    float map_width;
    float map_height;
    float map_half_width;
    float map_half_height;
public:
    std::vector<LveGameObject> wall_blocks;
   // std::unordered_map<std::pair<int32_t, int32_t>, LveGameObject*, pair_hash> wall_spatial_map;
    std::vector<std::vector<int32_t>> spatial_map;
    GameMaze() : maze_valid(false) {}
    ~GameMaze(void) {}

    std::pair<int32_t, int32_t> world_coords_to_indices(float x, float y) {
        // Assumes map is valid!
        if (x < -map_half_width || x >= map_half_width) {
            return {-1, -1};
        }
        if (y < -map_half_height || y >= map_half_height) {
            return {-1, -1};
        }
        int32_t x_int = x + map_half_width;
        int32_t y_int = y + map_half_height;

        return {x_int, y_int};
    }
    void generateMazeFromBoolVec(
        VKDeviceManager& device,
        std::vector<std::vector<bool>>& map
    ) {
        std::shared_ptr<VKModel> maze_wall_model =
            VKModel::createModelFromFile(device, "resources/models/cube.obj");

        map_height = float(map.size());
        map_width = float(map[0].size()); // Assuming all rows are the same size

        // Centering maze around 0 (for now)
        float h_mid = map_half_height = map_height / 2.f;
        float w_mid = map_half_width  = map_width / 2.f;

        glm::vec3 coord = {-w_mid, 0.f - 100*epsilon, -h_mid};
        for (int32_t y = 0; y < map.size(); y++) {
            spatial_map.push_back(std::vector<int32_t>(map_width));

            for (int32_t x = 0; x < map[y].size(); x++) {
                bool cell = map[y][x];
                if (cell) {
                    LveGameObject&& wall = LveGameObject::createGameObject();
                    wall.model = maze_wall_model;
                    wall.transform.translation = coord;
                    wall.transform.scale = {0.5f, 1.f, 0.5f};
                    wall.transform.update_matrices();
                    wall_blocks.emplace_back(std::move(wall));

                    // // Coordinates are simply "floor(coord)"
                    // std::pair<int32_t, int32_t> map_coords = {
                    //     std::floor(coord.x),
                    //     std::floor(coord.z)
                    // };
                    spatial_map[y][x] = wall_blocks.size() - 1;
                } else {
                    spatial_map[y][x] = -1;
                }
                coord.x += 1.f;
            }
            coord.z += 1.f;
            coord.x = -h_mid;
        }
        maze_valid = true;
    }

    void exportMazeVisibleGeometry(
        VKDeviceManager& device,
        LveGameObject::Map& obj_map
    ) {
        // Only allowed if maze has already been generated
        if (!maze_valid) {
            throw std::runtime_error("exporteMazeVisibleGeometry called without a valid maze!");
        }
        std::shared_ptr<VKModel> maze_wall_model_hedge =
            VKModel::createModelFromFile(device, "resources/models/hedge2.obj", true, glm::vec3(0.3f, 0.8f, 0.2f));
        std::shared_ptr<VKModel> maze_wall_model_red =
            VKModel::createModelFromFile(device, "resources/models/lsys.obj", true, glm::vec3(1.f, 0.1f, 0.1f));
        std::shared_ptr<VKModel> maze_wall_model_green =
            VKModel::createModelFromFile(device, "resources/models/lsys.obj", true, glm::vec3(0.3f, 0.8f, 0.2f));
            std::shared_ptr<VKModel> maze_wall_base_model =
            VKModel::createModelFromFile(device, "resources/models/cube.obj", true, glm::vec3(0.6f, 0.4f, 0.2f));

        // std::for_each(
        //     wall_spatial_map.begin(),
        //     wall_spatial_map.end(),
        //       [&obj_map, maze_wall_model](
        //         std::pair<const std::pair<int32_t, int32_t>, LveGameObject*> p
        //     )

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distribution(0, 3);
        std::uniform_int_distribution<> dist2(0, 1);

        for (const auto& wall : wall_blocks) {
            LveGameObject&& geom_wall = LveGameObject::createGameObject();
            int type = dist2(gen)==0;
            if (type==0) {
                geom_wall.model = maze_wall_model_red;
                geom_wall.transform.scale = {0.1f, -0.085f, 0.1f};
                geom_wall.transform.translation = {geom_wall.transform.translation.x,
                                                   geom_wall.transform.translation.y + 0.9f,
                                                   geom_wall.transform.translation.z};
//            } else if (type==1) {
//                geom_wall.model = maze_wall_model_green;
            } else {
                geom_wall.model = maze_wall_model_hedge;
                geom_wall.transform.scale = {0.85f, -0.85f, 0.85f};
                geom_wall.transform.translation = {geom_wall.transform.translation.x,
                                                   geom_wall.transform.translation.y + 0.4f,
                                                   geom_wall.transform.translation.z};
            }
            geom_wall.transform = wall.transform;

//            geom_wall.transform.scale = {0.85f, -0.85f, 0.85f};
            int randomRot = distribution(gen);
//            geom_wall.transform.rotation = {0, glm::radians(90.f * randomRot), 0};

            geom_wall.transform.update_matrices();
            obj_map.emplace(geom_wall.getId(), std::move(geom_wall));

            // add stacked object
//            geom_wall = LveGameObject::createGameObject();
//            geom_wall.model = maze_wall_model_green;
//            geom_wall.transform = wall.transform;

//            geom_wall.transform.scale = {0.1f, -0.085f, 0.1f};
//            geom_wall.transform.translation = {geom_wall.transform.translation.x,
//                                               geom_wall.transform.translation.y + 0.9f,
//                                               geom_wall.transform.translation.z};
//            randomRot = distribution(gen);
//            geom_wall.transform.rotation = {0, glm::radians(90.f * randomRot), 0};

//            geom_wall.transform.update_matrices();
//            obj_map.emplace(geom_wall.getId(), std::move(geom_wall));

            // Add a litle patch of dirt below
            LveGameObject&& geom_base = LveGameObject::createGameObject();
            geom_base.model = maze_wall_base_model;
            geom_base.transform = wall.transform;

            geom_base.transform.scale = {geom_base.transform.scale.x,
                                         geom_base.transform.scale.y / 10.f,
                                         geom_base.transform.scale.z};
            geom_base.transform.translation = {geom_base.transform.translation.x,
                                               geom_base.transform.translation.y + 1.f,
                                               geom_base.transform.translation.z};

            geom_base.transform.update_matrices();
            obj_map.emplace(geom_base.getId(), std::move(geom_base));
       }

        //);
    }
};

#endif // MAZE_H
