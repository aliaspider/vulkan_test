
#include <stdlib.h>
#include <string.h>
#include "vulkan.h"

void descriptors_init(const context_t *vk, const buffer_t *ubo, const texture_t *tex, descriptor_t* desc)
{
   {
      const VkDescriptorPoolSize sizes[] =
      {
         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
      };

      const VkDescriptorPoolCreateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
         .maxSets = 1,
         .poolSizeCount = countof(sizes), sizes
      };
      vkCreateDescriptorPool(vk->device, &info, NULL, &desc->pool);
   }

   {
      const VkDescriptorSetLayoutBinding bindings[] =
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

      const VkDescriptorSetLayoutCreateInfo info [] =
      {
         {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = countof(bindings), bindings

         }
      };
      vkCreateDescriptorSetLayout(vk->device, &info[0], NULL, &desc->set_layout);
   }

   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = desc->pool,
         .descriptorSetCount = 1, &desc->set_layout
      };
      vkAllocateDescriptorSets(vk->device, &info, &desc->set);
   }

   {
      const VkDescriptorBufferInfo buffer_info =
      {
         .buffer = ubo->handle,
         .offset = 0,
         .range = ubo->size
      };
      const VkDescriptorImageInfo image_info =
      {
         .sampler = tex->sampler,
         .imageView = tex->view,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      };

      const VkWriteDescriptorSet write_set[] =
      {
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = desc->set,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &buffer_info
         },
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = desc->set,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &image_info
         }
      };
      vkUpdateDescriptorSets(vk->device, countof(write_set), write_set, 0, NULL);
   }
}

void descriptors_free(const context_t *vk, descriptor_t *desc)
{
   vkDestroyDescriptorPool(vk->device, desc->pool, NULL);
   vkDestroyDescriptorSetLayout(vk->device, desc->set_layout, NULL);
   desc->pool = VK_NULL_HANDLE;
   desc->set_layout = VK_NULL_HANDLE;
}
