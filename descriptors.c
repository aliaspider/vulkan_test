
#include <stdlib.h>
#include <string.h>
#include "vulkan.h"

void descriptors_init(const context_t *vk, const buffer_t *ubo, const texture_t *tex, descriptor_t* desc)
{
   VkDescriptorPoolSize descriptorPoolSizes[] =
   {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
   };

   VkDescriptorPoolCreateInfo descriptorPoolCreateInfo =
   {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = countof(descriptorPoolSizes), descriptorPoolSizes
   };

   vkCreateDescriptorPool(vk->device, &descriptorPoolCreateInfo, NULL, &desc->pool);

   VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[] =
   {
      {
         .binding = 0,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,

      },
      {
         .binding = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
//         .pImmutableSamplers = &vk.texture_sampler

      }
   };
   VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo [] =
   {
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .bindingCount = countof(descriptorSetLayoutBindings), descriptorSetLayoutBindings

      }
   };

   vkCreateDescriptorSetLayout(vk->device, &descriptorSetLayoutCreateInfo[0], NULL,
                               &desc->set_layout);

   VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
   {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = desc->pool,
      .descriptorSetCount = 1, &desc->set_layout
   };
   vkAllocateDescriptorSets(vk->device, &descriptorSetAllocateInfo, &desc->set);

   VkDescriptorBufferInfo descriptorBufferInfo =
   {
      .buffer = ubo->handle,
      .offset = 0,
      .range = ubo->size
   };
   VkDescriptorImageInfo descriptorImageInfo =
   {
      .sampler = tex->sampler,
      .imageView = tex->view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
   };

   VkWriteDescriptorSet writeDescriptorSet[] =
   {
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = desc->set,
         .dstBinding = 0,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .pBufferInfo = &descriptorBufferInfo
      },
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = desc->set,
         .dstBinding = 1,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .pImageInfo = &descriptorImageInfo
      }
   };

   vkUpdateDescriptorSets(vk->device, countof(writeDescriptorSet), writeDescriptorSet, 0, NULL);
}

void descriptors_free(const context_t *vk, descriptor_t *desc)
{
   vkDestroyDescriptorPool(vk->device, desc->pool, NULL);
   vkDestroyDescriptorSetLayout(vk->device, desc->set_layout, NULL);
   desc->pool = VK_NULL_HANDLE;
   desc->set_layout = VK_NULL_HANDLE;
}
