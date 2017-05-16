
#include "vulkan.h"

void texture_init(const context_t *vk, int width, int height, texture_t* tex)
{
   tex->width = width;
   tex->height = height;
   tex->dirty = true;
   {
      VkImageCreateInfo info =
      {
         VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
         .flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
         .imageType = VK_IMAGE_TYPE_2D,
         .format = VK_FORMAT_R8G8B8A8_SRGB,
         .extent.width = width,
         .extent.height = height,
         .extent.depth = 1,
         .mipLevels = 1,
         .arrayLayers = 1,
         .samples = VK_SAMPLE_COUNT_1_BIT,
         .tiling = VK_IMAGE_TILING_OPTIMAL,
         .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
         .queueFamilyIndexCount = 1,
         .pQueueFamilyIndices = &vk->queue_family_index,
         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
      };
      tex->layout = info.initialLayout;
      vkCreateImage(vk->device, &info, NULL, &tex->image);

      info.tiling = VK_IMAGE_TILING_LINEAR;
      info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
      info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
      tex->staging.layout = info.initialLayout;
      vkCreateImage(vk->device, &info, NULL, &tex->staging.image);
   }

   {
      VkImageSubresource imageSubresource =
      {
         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .mipLevel = 0,
         .arrayLayer = 0
      };
      vkGetImageSubresourceLayout(vk->device, tex->image, &imageSubresource, &tex->mem_layout);
      vkGetImageSubresourceLayout(vk->device, tex->staging.image, &imageSubresource, &tex->staging.mem_layout);
   }

   image_memory_init(vk, tex->image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &tex->mem);
   image_memory_init(vk, tex->staging.image, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &tex->staging.mem);

   {
      VkImageViewCreateInfo info =
      {
         VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image = tex->image,
         .viewType = VK_IMAGE_VIEW_TYPE_2D,
         .format = VK_FORMAT_R8G8B8A8_SRGB,
         .components.r = VK_COMPONENT_SWIZZLE_R,
         .components.g = VK_COMPONENT_SWIZZLE_G,
         .components.b = VK_COMPONENT_SWIZZLE_B,
         .components.a = VK_COMPONENT_SWIZZLE_A,
         .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .subresourceRange.levelCount = 1,
         .subresourceRange.layerCount = 1
      };
      vkCreateImageView(vk->device, &info, NULL, &tex->view);
   }

   {
      VkSamplerCreateInfo samplerCreateInfo =
      {
         VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
         .magFilter = VK_FILTER_LINEAR,
         .minFilter = VK_FILTER_LINEAR,
         .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
         .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
         .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
         .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
      };
      vkCreateSampler(vk->device, &samplerCreateInfo, NULL, &tex->sampler);
   }
}


void texture_free(const context_t *vk, texture_t *tex)
{
   vkDestroySampler(vk->device, tex->sampler, NULL);
   vkDestroyImageView(vk->device, tex->view, NULL);
   vkDestroyImage(vk->device, tex->image, NULL);
   vkDestroyImage(vk->device, tex->staging.image, NULL);
   memory_free(vk, &tex->mem);
   memory_free(vk, &tex->staging.mem);
   tex->sampler = VK_NULL_HANDLE;
   tex->view = VK_NULL_HANDLE;
   tex->image = VK_NULL_HANDLE;
}

void texture_update(const context_t* vk, VkCommandBuffer cmd, texture_t* tex)
{
   memory_flush(vk, &tex->staging.mem);

   VkImageMemoryBarrier barrier =
   {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
      .oldLayout = tex->staging.layout,
      .newLayout = VK_IMAGE_LAYOUT_GENERAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image  = tex->staging.image,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.levelCount = 1,
      .subresourceRange.layerCount = 1
   };
   tex->staging.layout = barrier.newLayout;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

   barrier.srcAccessMask = 0;
   barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
   barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   barrier.image  = tex->image;
   tex->layout = barrier.newLayout;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

   {
      const VkImageCopy copy =
      {
         .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .srcSubresource.layerCount = 1,
         .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .dstSubresource.layerCount = 1,
         .extent.width = tex->width,
         .extent.height = tex->height,
         .extent.depth = 1
      };
      vkCmdCopyImage(cmd, tex->staging.image, tex->staging.layout, tex->image, tex->layout, 1, &copy);
   }

   barrier.srcAccessMask = barrier.dstAccessMask;
   barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
   barrier.oldLayout = barrier.newLayout;
   barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   tex->layout = barrier.newLayout;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

   tex->dirty = false;
}
