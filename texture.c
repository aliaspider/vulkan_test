
#include "vulkan.h"

void texture_init(const context_t *vk, int width, int height, texture_t* tex)
{
   tex->width = width;
   tex->height = height;
   VkImageCreateInfo imageCreateInfo =
   {
      VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
//      .flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .extent.width = width,
      .extent.height = height,
      .extent.depth = 1,
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_LINEAR,
      .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = &vk->queue_family_index,
      .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
   };
   vkCreateImage(vk->device, &imageCreateInfo, NULL, &tex->image);

   VkImageSubresource imageSubresource =
   {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel = 0,
      .arrayLayer = 0
   };
   vkGetImageSubresourceLayout(vk->device, tex->image, &imageSubresource, &tex->layout);

   image_memory_init(vk, tex->image, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &tex->mem);

   VkImageViewCreateInfo imageViewCreateInfo =
   {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = tex->image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.levelCount = 1,
      .subresourceRange.layerCount = 1
   };
   vkCreateImageView(vk->device, &imageViewCreateInfo, NULL, &tex->view);

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


void texture_free(const context_t *vk, texture_t *tex)
{
   vkDestroySampler(vk->device, tex->sampler, NULL);
   vkDestroyImageView(vk->device, tex->view, NULL);
   vkDestroyImage(vk->device, tex->image, NULL);
   memory_free(vk, &tex->mem);
   tex->sampler = VK_NULL_HANDLE;
   tex->view = VK_NULL_HANDLE;
   tex->image = VK_NULL_HANDLE;
}
