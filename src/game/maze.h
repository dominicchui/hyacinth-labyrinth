#ifndef MAZE_H
#define MAZE_H

#include <optional>
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
    std::vector<std::vector<bool>> boolVec;

//    std::vector<std::tuple<std::string, float>> large_assets = {
//        std::tuple("tree ", 0.5),
//        std::tuple("lake ", 0.4)
//    };
//    wall_assets = {
//              '__/-' : 0,
//              '----' : 1,
//              '-\__' : 2
//          }


//    std::tuple<int, int> findsq(auto mz) {
//        for (int ri = 0; ri < mz[0].size() - 2; ri++) {
//            for (int ci = 0; ci < mz.size() - 2; ci++) {
//                bool val = mz[ri][ci];
//                if (val == 1 and val == mz[ri + 1][ci] and val == mz[ri][ci + 1] and val == mz[ri + 1][ci + 1] and
//                    val == mz[ri][ci + 2] and val == mz[ri + 1][ci + 2] and val == mz[ri + 2][ci + 2] and val == mz[ri + 2][ci] and val == mz[ri + 2][ci + 1]) {
//                    return std::tuple(ri, ci);
//                }
//            }
//        }
//        return std::tuple(-1, -1);
//    }

    std::optional<bool> getOOB(std::vector<std::vector<bool>> mz, int r, int c) {
        if (r < 0 or c < 0 or r >= mz.size() or c >= mz[0].size()) {
            return std::optional<bool>();
        }
        return mz[r][c];
    }

//    std::optional<std::string> getOOB(std::vector<std::vector<std::string>> mz, int r, int c) {
//        if (r < 0 or c < 0 or r >= mz.size() or c >= mz[0].size()) {
//            return std::optional<std::string>();
//        }
//        return mz[r][c];
//    }

    float getOOBpop(auto mz, int r, int c) {
        std::optional<float> mzVal = getOOB(mz, r, c);
        return mzVal.has_value() ? (mzVal.value() - 0.4) * 3.f : 0;
    }

    float avgpop(auto mz, int ri, int ci) {
        float sum = 0;
        for (int i = -2; i <= 2; i++) {
            for (int j = -2; j <= 2; j++) {
                sum += getOOBpop(mz, ri + i, ci + j);
            }
        }
        return sum / 25.f;
    }

    std::vector<std::vector<float>> populate(std::vector<std::vector<bool>> mz) {
        std::vector<std::vector<float>> output = {};
        for (int ri = 0; ri < mz.size(); ri++) {
            std::vector<float> outr = {};
            for (int ci = 0; ci < mz[ri].size(); ci++) {
                if (mz[ri][ci]) {
                    outr.push_back(avgpop(mz, ri, ci));
                } else {
                    outr.push_back(0);
                }
            }
            output.push_back(outr);
        }
        return output;
    }

    std::string get_asset(std::vector<std::tuple<std::string, float>> alist, float p) {
        std::string clA = "    ";
        float clV = 99;
        for (int i = 0; i < alist.size(); i++) {
            auto [asset, value] = alist[i];
            float dist = abs(value - p);
            if (dist < clV) {
                clV = dist;
                clA = asset;
            }
        }
        return clA;
    }

    //void find_stretch(std::vector<std::vector<std::string>> filled) {
    //    int width = filled[0].size();
    //    std::vector<std::tuple<int, int>> found = {};
    //    for (int ri = 0; ri < filled.size(); ri++) {
    //        for (int ci = 0; ci < filled[ri].size(); ci++) {
    //            if (filled[ri][ci] == "UNFILLED" and (getOOB(filled, ri, ci - 1) == "UNFILLED") and (getOOB(filled, ri, ci + 1) == "UNFILLED")) {
    //                found.push_back(std::tuple(ri, ci));
    //            }
    //        }
    //    }
    //    if (!found.empty()) {
    //        ri, ci = choice(found)
    //        for i in range(ci, width, 1):
    //          filled[ri][i] = '---- '
    //          if i == width-1 or filled[ri][i+1] != 'UNFILLED':
    //            filled[ri][i] = '-\__ '
    //            break
    //        for i in range(ci, -1, -1):
    //          filled[ri][i] = '---- '
    //          if i == 0 or filled[ri][i-1] != 'UNFILLED':
    //            filled[ri][i] = '__/- '
    //            break
    //    }
    //}

    std::vector<std::vector<std::string>> fill_assets(std::vector<std::vector<bool>> mz, std::vector<std::vector<float>> pop) {

        std::vector<std::vector<std::string>> output = {};
        for (int r = 0; r < mz.size(); r++) {
            std::vector<std::string> o = {};
            for (int c = 0; c < mz[0].size(); c++) {
                if (mz[r][c]) {
                    o.push_back("UNFILLED");
                } else {
//                    if (rand() < 0.05) {
//                        o.push_back("perg ");
//                    } else {
                        o.push_back("     ");
//                    }
                }
            }
            output.push_back(o);
        }

//        auto [tr, tc] = findsq(mz);
//        if (tr != -1) {
//            std::string lg_asset = get_asset(large_assets, pop[tr][tc]);
//            for (int i = 0; i <= 2; i++) {
//                for (int j = 0; j <= 2; j++) {
//                    output[tr + i][tc + j] = lg_asset;
//                }
//            }
//        }
        //  wall stretches
        //  find_stretch(output)

        std::vector<std::tuple<std::string, float>> assets = {
            std::tuple("bush ", 0.35),
            std::tuple("fbush ", 0.17),
            std::tuple("tree ", 0.05)
        };

        for (int r = 0; r < output.size(); r++) {
            std::vector<std::string> o = {};
            for (int c = 0; c < output[0].size(); c++) {
                if (output[r][c] == "UNFILLED") {
                    output[r][c] = get_asset(assets, pop[r][c]);
                }
            }
        }

        return output;
    }

public:
    std::vector<LveGameObject> wall_blocks;
    std::vector<glm::vec4> path_coords;
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
        boolVec = map;
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
                    // Just store our coordinates to come back to later
                    glm::vec3 translation(coord);
                    // Store the parity for check tiling in the last entry of the vec4
                    float parity = (x+y) % 2;
                    path_coords.push_back(glm::vec4(translation, parity));
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
            VKModel::createModelFromFile(device, "resources/models/hilbush.obj");
        std::shared_ptr<VKModel> maze_flower_model =
            VKModel::createModelFromFile(device, "resources/models/big-color-flower-red.obj");
        std::shared_ptr<VKModel> maze_flower2_model =
            VKModel::createModelFromFile(device, "resources/models/big-color-flower-blue.obj");
        std::shared_ptr<VKModel> maze_tree1_model =
            VKModel::createModelFromFile(device, "resources/models/tree3.obj", true, glm::vec3(0.6f, 0.4f, 0.6f));
        std::shared_ptr<VKModel> maze_wall_base_model =
            VKModel::createModelFromFile(device, "resources/models/dirt.obj", true, glm::vec3(0.6f, 0.4f, 0.2f));
        std::shared_ptr<VKModel> path_base_model_0 =
            VKModel::createModelFromFile(device, "resources/models/tile.obj", true, glm::vec3(1.f, 1.f, 1.f));
//                                                                                  glm::vec3(153.f, 180.f, 255.f) / 255.f);
        std::shared_ptr<VKModel> path_base_model_1 =
            VKModel::createModelFromFile(device, "resources/models/tile.obj", true, glm::vec3(1.f, 1.f, 1.f));

        std::random_device rd;
        std::mt19937 gen(0);
        std::uniform_int_distribution<> distribution(0, 7);

        auto pop = populate(boolVec);
        auto asset_map = fill_assets(boolVec, pop);
        for (auto &a: asset_map) {
            for (auto &b:  a) {
                std::cout  << b;
            }
            std::cout <<  std::endl;
        }

        for (const auto& wall : wall_blocks) {

            int randomRot = distribution(gen);
            auto [mx, my] = world_coords_to_indices(wall.transform.translation.x, wall.transform.translation.z);

            if (asset_map[my][mx] == "fbush ") {
                // occasional flowers
                LveGameObject&& flower = LveGameObject::createGameObject();
                std::uniform_int_distribution<> distribution2(0, 1);
                if (distribution2(gen)==0) {
                    flower.model = maze_flower_model;
                } else {
                    flower.model = maze_flower2_model;

                }
                flower.transform = wall.transform;
                flower.transform.scale = {0.04f, -0.04f, 0.04f};
                flower.transform.translation = {flower.transform.translation.x,
                                                flower.transform.translation.y + 1.f,
                                                flower.transform.translation.z};
                flower.transform.rotation = {0, glm::radians(45.f * randomRot), 0};
                flower.transform.update_matrices();
                obj_map.emplace(flower.getId(), std::move(flower));

            } else if (asset_map[my][mx] == "tree ") {
                std::uniform_int_distribution<> distrib2(0, 1);
                LveGameObject&& geom_wall = LveGameObject::createGameObject();
                geom_wall.transform = wall.transform;
                geom_wall.model = maze_tree1_model;
                geom_wall.transform.scale = {0.15f, -0.2f, 0.15f};
                geom_wall.transform.translation = {geom_wall.transform.translation.x,
                                                   geom_wall.transform.translation.y + 0.9f,
                                                   geom_wall.transform.translation.z};
                geom_wall.transform.rotation = {0, glm::radians(45.f * randomRot), 0};
                geom_wall.transform.update_matrices();
                obj_map.emplace(geom_wall.getId(), std::move(geom_wall));
            } else {
                LveGameObject&& geom_wall = LveGameObject::createGameObject();
                geom_wall.model = maze_wall_model;
                geom_wall.transform = wall.transform;
                geom_wall.transform.scale = {0.085f, -0.085f, 0.085f};
                geom_wall.transform.translation = {geom_wall.transform.translation.x - 0.5f,
                                                   geom_wall.transform.translation.y + 0.9f,
                                                   geom_wall.transform.translation.z + 0.5f};
                //                geom_wall.transform.rotation = {0, glm::radians(90.f * randomRot), 0};
                geom_wall.transform.update_matrices();
                obj_map.emplace(geom_wall.getId(), std::move(geom_wall));
            }
            // always add hedge
            LveGameObject&& geom_wall = LveGameObject::createGameObject();
            geom_wall.model = maze_wall_model;
            geom_wall.transform = wall.transform;
            geom_wall.transform.scale = {0.085f, -0.085f, 0.085f};
            geom_wall.transform.translation = {geom_wall.transform.translation.x - 0.5f,
                                               geom_wall.transform.translation.y + 0.9f,
                                               geom_wall.transform.translation.z + 0.5f};
            //                geom_wall.transform.rotation = {0, glm::radians(90.f * randomRot), 0};
            geom_wall.transform.update_matrices();
            obj_map.emplace(geom_wall.getId(), std::move(geom_wall));

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

       for (const auto &coord : path_coords) {
            // Add a tile
            LveGameObject&& geom_tile = LveGameObject::createGameObject();
            int parity = coord[3];
            if (parity == 0)
                geom_tile.model = path_base_model_0;
            else
                geom_tile.model = path_base_model_1;

            geom_tile.transform.scale = {0.47f, 1.f, 0.47f};
            geom_tile.transform.translation = glm::vec3(coord);

            geom_tile.transform.scale = {geom_tile.transform.scale.x,
                                         geom_tile.transform.scale.y / 10.f,
                                         geom_tile.transform.scale.z};
            geom_tile.transform.translation = {geom_tile.transform.translation.x,
                                               geom_tile.transform.translation.y + 1.f,
                                               geom_tile.transform.translation.z};

            geom_tile.transform.update_matrices();
            obj_map.emplace(geom_tile.getId(), std::move(geom_tile));
       }
    }
};

#endif // MAZE_H
