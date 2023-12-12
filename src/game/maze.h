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
        std::shared_ptr<VKModel> maze_wall_model =
            VKModel::createModelFromFile(device, "resources/models/hedge.obj");

        // std::for_each(
        //     wall_spatial_map.begin(),
        //     wall_spatial_map.end(),
        //       [&obj_map, maze_wall_model](
        //         std::pair<const std::pair<int32_t, int32_t>, LveGameObject*> p
        //     )

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distribution(0, 90);

        for (const auto& wall : wall_blocks) {
            LveGameObject&& geom_wall = LveGameObject::createGameObject();
            geom_wall.model = maze_wall_model;
            geom_wall.transform = wall.transform;

            geom_wall.transform.scale = {0.13f, -0.13f, 0.13f};
            int randomRot = distribution(gen);
            geom_wall.transform.rotation = {0, randomRot, 0};

            geom_wall.transform.update_matrices();
            obj_map.emplace(geom_wall.getId(), std::move(geom_wall));
       }

        //);
    }
};

#endif // MAZE_H
