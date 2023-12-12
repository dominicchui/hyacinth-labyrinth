#pragma once

#include "vulkan-buffer.hpp"
#include "vulkan-device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

class VKModel {
  public:
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex &other) const {
            return position == other.position && color == other.color && normal == other.normal &&
                   uv == other.uv;
        }
    };

    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        void loadModelWithMaterial(const std::string &filepath);
        void loadModel(const std::string &filepath);
    };

    VKModel(VKDeviceManager& device, const VKModel::Builder &builder);
    ~VKModel();

    VKModel(const VKModel&) = delete;
    VKModel &operator=(const VKModel &) = delete;

  static std::unique_ptr<VKModel> createModelFromFile(
      VKDeviceManager& device,
      const std::string& filepath,
      bool override_color = false,
      glm::vec3 color = glm::vec3(0.f,0.f,0.f)
  );

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

    VkImageView textureImageView;
    VkSampler textureSampler;

  private:
    void createImage(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory
    );

    void createTextureImage(void);
    void createTextureImageView(void);
    void createTextureSampler(void);
    void createVertexBuffers(const std::vector<Vertex>& vertices);
    void createIndexBuffers(const std::vector<uint32_t>& indices);

    VKDeviceManager& m_device;

    std::unique_ptr<VKBufferMgr> vertexBuffer;
    uint32_t vertexCount;

    bool hasIndexBuffer = false;
    std::unique_ptr<VKBufferMgr> indexBuffer;
    uint32_t indexCount;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
};
