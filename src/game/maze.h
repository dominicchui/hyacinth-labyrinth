#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

#include "vulkan/vulkan-model.hpp"
#include "vulkan/vulkan-device.hpp"
#include "utils/utils.h"
#include "lve_game_object.hpp"

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (std::pair<T1, T2> const &v) const
    {
        int64_t val = int64_t(v.first) & (int64_t(v.second) << 32);
        return std::hash<T1>()(val);
    }
};

class MazeBlock {
private:
    bool maze_valid;
    float map_width;
    float map_height;
    std::unordered_map<std::pair<int32_t, int32_t>, LveGameObject, pair_hash> wall_spatial_map;
public:
    MazeBlock() : maze_valid(false), wall_spatial_map() {}
    ~MazeBlock(void) {}

    void generateMazeFromBoolVec(
        VKDeviceManager& device,
        std::vector<std::vector<bool>>& map
    ) {
        std::shared_ptr<VKModel> maze_wall_model =
            VKModel::createModelFromFile(device, "resources/models/cube.obj");

        map_height = float(map.size());
        map_width = float(map[0].size()); // Assuming all rows are the same size

        // Centering maze around 0 (for now)
        float h_mid = map_height / 2.f;
        float w_mid = map_width / 2.f;

        glm::vec3 coord = {-h_mid, 0.f - 100*epsilon, -w_mid};
        for (const std::vector<bool>& row : map) {
            for (const bool& cell : row) {
                if (cell) {
                    auto wall = LveGameObject::createGameObject();
                    wall.model = maze_wall_model;
                    wall.transform.translation = coord;
                    wall.transform.scale = {0.5f, 1.f, 0.5f};
                    wall.transform.update_matrices();

                    // Coordinates are simply "floor(coord)"
                    std::pair<int32_t, int32_t> map_coords = {
                        std::floor(coord.x),
                        std::floor(coord.z)
                    };
                    wall_spatial_map.emplace(map_coords, std::move(wall));
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
            VKModel::createModelFromFile(device, "resources/models/cube.obj");

        glm::vec4 cyan = {0,1,1,1};
        glm::mat4 flctm = {{9.000000, 0.000000, 0.000000, 0.000000}, {0.000000, 0.100000, 0.000000, 0.000000}, {0.000000, 0.000000, 9.000000, 0.000000}, {0.000000, -0.900000, 0.000000, 1.000000}};

        glm::vec4 yellow = {1,1,0,1};

        std::for_each(
            wall_spatial_map.begin(),
            wall_spatial_map.end(),
              [&obj_map, maze_wall_model](
                std::pair<const std::pair<int32_t, int32_t>, LveGameObject&> p
            ) {
                auto wall = LveGameObject::createGameObject();
                wall.model = maze_wall_model;
                wall.transform = p.second.transform;

                obj_map.emplace(wall.getId(), std::move(wall));
            }
        );
    }
};

#endif // MAZE_H
