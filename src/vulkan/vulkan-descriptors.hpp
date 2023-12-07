#pragma once

#include "vulkan/vulkan-device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

class VK_DSL_Mgr {
 public:
  class Builder {
   public:
    Builder(VKDeviceManager& device) : m_device(device) {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<VK_DSL_Mgr> build() const;

   private:
    VKDeviceManager& m_device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings{};
  };

  VK_DSL_Mgr(
      VKDeviceManager& device,
      std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings
  );

  ~VK_DSL_Mgr();
  VK_DSL_Mgr(const VK_DSL_Mgr &) = delete;
  VK_DSL_Mgr &operator=(const VK_DSL_Mgr &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
  VKDeviceManager& m_device;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

  friend class VKDescriptorWriter;
};

class VK_DP_Mgr {
 public:
  class Builder {
   public:
    Builder(VKDeviceManager &device) : m_device(device) {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<VK_DP_Mgr> build() const;

   private:
    VKDeviceManager &m_device;
    std::vector<VkDescriptorPoolSize> m_poolSizes{};
    uint32_t m_maxSets = 1000;
    VkDescriptorPoolCreateFlags m_poolFlags = 0;
  };

  VK_DP_Mgr(
      VKDeviceManager& device,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~VK_DP_Mgr();
  VK_DP_Mgr(const VK_DP_Mgr&) = delete;
  VK_DP_Mgr& operator=(const VK_DP_Mgr &) = delete;

  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

 private:
  VKDeviceManager& m_device;
  VkDescriptorPool descriptorPool;

  friend class VKDescriptorWriter;
};

class VKDescriptorWriter {
 public:
  VKDescriptorWriter(VK_DSL_Mgr& setLayout, VK_DP_Mgr &pool);

  VKDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  VKDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  VK_DSL_Mgr& m_setLayout;
  VK_DP_Mgr& m_pool;
  std::vector<VkWriteDescriptorSet> m_writes;
};
