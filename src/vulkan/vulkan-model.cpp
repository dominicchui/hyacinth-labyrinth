#include "utils/debug.h"
#include "vulkan-model.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <extern/tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <extern/stb_image.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <glm/glm.hpp>

// std
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <filesystem>
#include <iostream>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
static void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
};

template <>
struct std::hash<VKModel::Vertex> {
  size_t operator()(VKModel::Vertex const &vertex) const {
    size_t seed = 0;
    hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};

VKModel::VKModel(
    VKDeviceManager& device,
    const VKModel::Builder& builder)
    : m_device(device)
{
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

VKModel::~VKModel() {
    vkDestroySampler(m_device.device(), textureSampler, nullptr);
    vkDestroyImageView(m_device.device(), textureImageView, nullptr);
    vkDestroyImage(m_device.device(), textureImage, nullptr);
    vkFreeMemory(m_device.device(), textureImageMemory, nullptr);
}

void VKModel::createImage(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory
) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    m_device.createImageWithInfo(
        imageInfo,
        properties,
        image,
        imageMemory
    );
}

void VKModel::createTextureImage(void) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("../resources/textures/andyVanDam.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VKBufferMgr stagingBuffer{
        m_device,
        imageSize,
        1,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    // Copy image data to a buffer
    stagingBuffer.map();
    stagingBuffer.writeToBuffer(pixels);
    stagingBuffer.unmap();

    stbi_image_free(pixels);

    createImage(
        texWidth,
        texHeight,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        textureImage,
        textureImageMemory
    );

    // Change the texture image layout
    m_device.transitionImageLayout(textureImage,
                                   VK_FORMAT_R8G8B8A8_SRGB,
                                   VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // Copy to the destination buffer
    m_device.copyBufferToImage(stagingBuffer.getBuffer(),
                               textureImage,
                               static_cast<uint32_t>(texWidth),
                               static_cast<uint32_t>(texHeight),
                               1);

    // Cleanup
    m_device.transitionImageLayout(textureImage,
                                   VK_FORMAT_R8G8B8A8_SRGB,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VKModel::createTextureSampler(void) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = m_device.properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void VKModel::createTextureImageView() {
    textureImageView =
        m_device.createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

std::unique_ptr<VKModel> VKModel::createModelFromFile(
    VKDeviceManager& device,
    const std::string& filepath,
    bool with_material
) {
    Builder builder{};
    // if (with_material) {
    //     builder.loadModelWithMaterial(ENGINE_DIR + filepath);
    // } else {
        builder.loadModel(ENGINE_DIR + filepath);
    // }
    return std::make_unique<VKModel>(device, builder);
}

void VKModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  uint32_t vertexSize = sizeof(vertices[0]);

  VKBufferMgr stagingBuffer{
      m_device,
      vertexSize,
      vertexCount,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  vertexBuffer = std::make_unique<VKBufferMgr>(
      m_device,
      vertexSize,
      vertexCount,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  m_device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void VKModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
  uint32_t indexSize = sizeof(indices[0]);

  VKBufferMgr stagingBuffer{
      m_device,
      indexSize,
      indexCount,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  indexBuffer = std::make_unique<VKBufferMgr>(
      m_device,
      indexSize,
      indexCount,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  m_device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void VKModel::draw(VkCommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

void VKModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
  }
}

std::vector<VkVertexInputBindingDescription> VKModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VKModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
  attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
  attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
  attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

  return attributeDescriptions;
}

static void populate_pos(const tinyobj::index_t& index, const auto& attrib, VKModel::Vertex& vertex) {
    if (index.vertex_index >= 0) {
        vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2],
        };

        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
        };
    }
}

static void populate_normals(
    const tinyobj::index_t& idx_x,
    const tinyobj::index_t& idx_y,
    const tinyobj::index_t& idx_z,
    const auto& attrib,
    VKModel::Vertex& vertex_x,
    VKModel::Vertex& vertex_y,
    VKModel::Vertex& vertex_z
) {
    const glm::vec3& a = vertex_x.position;
    const glm::vec3& b = vertex_y.position;
    const glm::vec3& c = vertex_z.position;

    glm::vec3 edge_xy = b - a;
    glm::vec3 edge_xz = c - a;
    glm::vec3 edge_yz = c - b;

    if (idx_x.normal_index >= 0) {
        vertex_x.normal = {
            attrib.normals[3 * idx_x.normal_index + 0],
            attrib.normals[3 * idx_x.normal_index + 1],
            attrib.normals[3 * idx_x.normal_index + 2],
        };
    } else {
        vertex_x.normal = glm::normalize(glm::cross(edge_xy, edge_xz));
    }

    if (idx_y.normal_index >= 0) {
        vertex_y.normal = {
            attrib.normals[3 * idx_y.normal_index + 0],
            attrib.normals[3 * idx_y.normal_index + 1],
            attrib.normals[3 * idx_y.normal_index + 2],
        };
    } else {
        vertex_y.normal = glm::normalize(glm::cross(edge_yz, -edge_xy));
    }

    if (idx_z.normal_index >= 0) {
        vertex_z.normal = {
            attrib.normals[3 * idx_z.normal_index + 0],
            attrib.normals[3 * idx_z.normal_index + 1],
            attrib.normals[3 * idx_z.normal_index + 2],
        };
    } else {
        vertex_z.normal = glm::normalize(glm::cross(-edge_xz, -edge_yz));
    }

}

static void populate_tex_coords(const tinyobj::index_t& idx, const auto& attrib, VKModel::Vertex& vertex) {
    if (idx.texcoord_index >= 0) {
        vertex.uv = {
            attrib.texcoords[2 * idx.texcoord_index + 0],
            attrib.texcoords[2 * idx.texcoord_index + 1],
        };
    }
}

static void printVertex(const VKModel::Vertex& v, int32_t idx) {
#if DEBUG_LEVEL > 2
    std::cout << "Vertex #" << idx << std::endl;
    std::cout << "\tpos ";
    printVec3(v.position, false);
    std::cout << "\tclr ";
    printVec3(v.color, false);
    std::cout << "\tnor ";
    printVec3(v.normal, false);
    std::cout << "\tuv ";
    printVec2(v.uv, true);
#endif
    return;
}

void VKModel::Builder::loadModel(const std::string &filepath) {
    namespace fs = std::filesystem;
    fs::path obj_path(filepath);
    std::string parent_dir = obj_path.parent_path().string();

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = parent_dir;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        if (!reader.Error().empty()) {
            throw std::runtime_error("TinyObjReader: " + reader.Error());
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto &shape : shapes) {
        for (int32_t i = 0; i < shape.mesh.indices.size(); i++) {
            assert(shape.mesh.num_face_vertices[i/3] == uint8_t(3));

            tinyobj::index_t idx_x = shape.mesh.indices[i];
            tinyobj::index_t idx_y = shape.mesh.indices[i+1];
            tinyobj::index_t idx_z = shape.mesh.indices[i+2];

            Vertex vertex_x{};
            Vertex vertex_y{};
            Vertex vertex_z{};

            // Positions and colors
            populate_pos(idx_x, attrib, vertex_x);
            populate_pos(idx_y, attrib, vertex_y);
            populate_pos(idx_z, attrib, vertex_z);

            // Normals
            populate_normals(idx_x, idx_y, idx_z, attrib, vertex_x, vertex_y, vertex_z);

            // UVs
            populate_tex_coords(idx_x, attrib, vertex_x);
            populate_tex_coords(idx_y, attrib, vertex_y);
            populate_tex_coords(idx_z, attrib, vertex_z);

            printVertex(vertex_x, idx_x.vertex_index);
            printVertex(vertex_y, idx_y.vertex_index);
            printVertex(vertex_z, idx_z.vertex_index);


            // Update render indices
            if (uniqueVertices.count(vertex_x) == 0) {
                uniqueVertices[vertex_x] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex_x);
            }
            indices.push_back(uniqueVertices[vertex_x]);

            if (uniqueVertices.count(vertex_y) == 0) {
                uniqueVertices[vertex_y] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex_y);
            }
            indices.push_back(uniqueVertices[vertex_y]);

            if (uniqueVertices.count(vertex_z) == 0) {
                uniqueVertices[vertex_z] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex_z);
            }
            indices.push_back(uniqueVertices[vertex_z]);

            // We deal with triangles here
            i += 2;
        }

    //     // Second pass to calculate normals

    //     if (attrib.normals.empty()) {
    //         // Compute normals per vertex
    //         std::vector<int32_t> num_normal_contribs(vertices.size(), 0);

    //         for (int32_t i = 0; i < shape.mesh.indices.size(); i++) {
    //             assert(shape.mesh.num_face_vertices[i/3] == uint8_t(3));

    //             tinyobj::index_t idx_x = shape.mesh.indices[i];
    //             tinyobj::index_t idx_y = shape.mesh.indices[i+1];
    //             tinyobj::index_t idx_z = shape.mesh.indices[i+2];

    //             glm::vec3 face_x = glm::vec3(attrib.vertices[3 * idx_x.vertex_index + 0],
    //                                          attrib.vertices[3 * idx_x.vertex_index + 1],
    //                                          attrib.vertices[3 * idx_x.vertex_index + 2]);
    //             glm::vec3 face_y = glm::vec3(attrib.vertices[3 * idx_y.vertex_index + 0],
    //                                          attrib.vertices[3 * idx_y.vertex_index + 1],
    //                                          attrib.vertices[3 * idx_y.vertex_index + 2]);
    //             glm::vec3 face_z = glm::vec3(attrib.vertices[3 * idx_z.vertex_index + 0],
    //                                          attrib.vertices[3 * idx_z.vertex_index + 1],
    //                                          attrib.vertices[3 * idx_z.vertex_index + 2]);


    //             glm::vec3 edge_xy = face_y - face_x;
    //             glm::vec3 edge_xz = face_z - face_x;
    //             glm::vec3 edge_yz = face_z - face_y;

    //             normals[face.x] += glm::normalize(glm::cross(edge_xy, edge_xz));
    //             num_normal_contribs[face.x] += 1;
    //             normals[face.y] += glm::normalize(glm::cross(edge_yz, -edge_xy));
    //             num_normal_contribs[face.y] += 1;
    //             normals[face.z] += glm::normalize(glm::cross(-edge_xz, -edge_yz));
    //             num_normal_contribs[face.z] += 1;
    //         }

    //         for (int32_t i = 0; i < normals.size(); i++) {
    //             normals[i] /= num_normal_contribs[i];
    //         }
    //     } else if (normals_in.size() != vertices.size()) {
    //         throw std::runtime_error("Obj file has a different number of normals than vertices");
    //     } else {
    //         std::memcpy(normals.data(), normals_in.data(), normals_in.size() * sizeof(glm::vec3));
    //     }

    //     // Fill vertex data
    //     for (int32_t i = 0; i < faces.size(); i++) {
    //         const glm::vec3& face = faces[i];
    //         insertVec3(m_vertexData, vertices[face.x]);
    //         insertVec3(m_vertexData, normals[face.x]);
    //         //insertVec3(m_vertexData, glm::vec3(0.f, 1.f, 0.f));

    //         insertVec3(m_vertexData, vertices[face.y]);
    //         insertVec3(m_vertexData, normals[face.y]);
    //         //insertVec3(m_vertexData, glm::vec3(1.f, 0.f, 0.f));


    //         insertVec3(m_vertexData, vertices[face.z]);
    //         insertVec3(m_vertexData, normals[face.z]);
    //         //insertVec3(m_vertexData, glm::vec3(0.f, 0.f, 1.f));



    //     }
    // }
    }
}

// void VKModel::Builder::loadModel(const std::string &filepath) {
//   tinyobj::attrib_t attrib;
//   std::vector<tinyobj::shape_t> shapes;
//   std::vector<tinyobj::material_t> materials;
//   std::string warn, err;

//   if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
//     throw std::runtime_error(warn + err);
//   }

//   vertices.clear();
//   indices.clear();

//   std::unordered_map<Vertex, uint32_t> uniqueVertices{};
//   for (const auto &shape : shapes) {
//     for (const auto &index : shape.mesh.indices) {
//       Vertex vertex{};

//       if (index.vertex_index >= 0) {
//         vertex.position = {
//             attrib.vertices[3 * index.vertex_index + 0],
//             attrib.vertices[3 * index.vertex_index + 1],
//             attrib.vertices[3 * index.vertex_index + 2],
//         };

//         vertex.color = {
//             attrib.colors[3 * index.vertex_index + 0],
//             attrib.colors[3 * index.vertex_index + 1],
//             attrib.colors[3 * index.vertex_index + 2],
//         };
//       }

//       if (index.normal_index >= 0) {
//         vertex.normal = {
//             attrib.normals[3 * index.normal_index + 0],
//             attrib.normals[3 * index.normal_index + 1],
//             attrib.normals[3 * index.normal_index + 2],
//         };
//       }

//       if (index.texcoord_index >= 0) {
//         vertex.uv = {
//             attrib.texcoords[2 * index.texcoord_index + 0],
//             attrib.texcoords[2 * index.texcoord_index + 1],
//         };
//       }

//       if (uniqueVertices.count(vertex) == 0) {
//         uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
//         vertices.push_back(vertex);
//       }
//       indices.push_back(uniqueVertices[vertex]);
//     }
//   }
// }
