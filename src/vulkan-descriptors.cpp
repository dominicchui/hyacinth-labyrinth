#include "vulkan-descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

// *************** Descriptor Set Layout Builder *********************

VK_DSL_Mgr::Builder &VK_DSL_Mgr::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count
) {
  assert(m_bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  m_bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<VK_DSL_Mgr> VK_DSL_Mgr::Builder::build() const {
  return std::make_unique<VK_DSL_Mgr>(m_device, m_bindings);
}

// *************** Descriptor Set Layout *********************

VK_DSL_Mgr::VK_DSL_Mgr(
    VKDeviceManager &device,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings
  ) : m_device(device), m_bindings(bindings)
{
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : m_bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  if (vkCreateDescriptorSetLayout(
          m_device.device(),
          &descriptorSetLayoutInfo,
          nullptr,
          &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

VK_DSL_Mgr::~VK_DSL_Mgr() {
  vkDestroyDescriptorSetLayout(m_device.device(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

VK_DP_Mgr::Builder& VK_DP_Mgr::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count
) {
  m_poolSizes.push_back({descriptorType, count});
  return *this;
}

VK_DP_Mgr::Builder& VK_DP_Mgr::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags
) {
  m_poolFlags = flags;
  return *this;
}
VK_DP_Mgr::Builder& VK_DP_Mgr::Builder::setMaxSets(uint32_t count) {
  m_maxSets = count;
  return *this;
}

std::unique_ptr<VK_DP_Mgr> VK_DP_Mgr::Builder::build() const {
  return std::make_unique<VK_DP_Mgr>(m_device, m_maxSets, m_poolFlags, m_poolSizes);
}

// *************** Descriptor Pool *********************

VK_DP_Mgr::VK_DP_Mgr(
    VKDeviceManager& device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : m_device{device}
{
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(m_device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

VK_DP_Mgr::~VK_DP_Mgr() {
  vkDestroyDescriptorPool(m_device.device(), descriptorPool, nullptr);
}

bool VK_DP_Mgr::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet &descriptor
) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  if (vkAllocateDescriptorSets(m_device.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
}

void VK_DP_Mgr::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(
      m_device.device(),
      descriptorPool,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data()
  );
}

void VK_DP_Mgr::resetPool() {
  vkResetDescriptorPool(m_device.device(), descriptorPool, 0);
}

// *************** Descriptor Writer *********************

VKDescriptorWriter::VKDescriptorWriter(
    VK_DSL_Mgr& setLayout,
    VK_DP_Mgr& pool
    ) : m_setLayout(setLayout), m_pool(pool) {}

VKDescriptorWriter& VKDescriptorWriter::writeBuffer(
    uint32_t binding,
    VkDescriptorBufferInfo* bufferInfo
) {
  assert(m_setLayout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = m_setLayout.m_bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  m_writes.push_back(write);
  return *this;
}

VKDescriptorWriter &VKDescriptorWriter::writeImage(
    uint32_t binding,
    VkDescriptorImageInfo *imageInfo
) {
  assert(m_setLayout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = m_setLayout.m_bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;

  m_writes.push_back(write);
  return *this;
}

bool VKDescriptorWriter::build(VkDescriptorSet &set) {
  bool success = m_pool.allocateDescriptor(m_setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void VKDescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : m_writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(m_pool.m_device.device(), m_writes.size(), m_writes.data(), 0, nullptr);
}
