#pragma once

#include <stdbool.h>
#include <vulkan/vulkan.h>

#define MAX_SWAPCHAIN_IMAGES 8
#define countof(a) (sizeof(a)/ sizeof(*a))
#define VULKAN_CALL(fname, instance, ...)                                      \
  ((PFN_##fname)vkGetInstanceProcAddr(instance, #fname))(instance, __VA_ARGS__)

typedef struct
{
   VkInstance instance;
   VkDebugReportCallbackEXT debug_cb;
   VkPhysicalDevice gpu;
   VkPhysicalDeviceMemoryProperties mem;
   VkDevice device;
   uint32_t queue_family_index;
   VkQueue queue;
   VkCommandPool cmd_pool;
}context_t;

typedef struct
{
   VkSurfaceKHR handle;
   int width;
   int height;
#ifdef VK_USE_PLATFORM_XLIB_KHR
   Display *display;
   Window   window;
#endif
}surface_t;

typedef struct
{
   VkSwapchainKHR handle;
   VkRect2D scissor;
   VkViewport viewport;
   VkRenderPass renderpass;
   uint32_t count;
   VkImageView views[MAX_SWAPCHAIN_IMAGES];
   VkFramebuffer framebuffers[MAX_SWAPCHAIN_IMAGES];
}swapchain_t;

typedef struct
{
   VkDeviceMemory handle;
   VkMemoryPropertyFlags flags;
   VkDeviceSize size;
   VkDeviceSize alignment;
   union
   {
      void* ptr;
      uint8_t* u8;
      uint32_t* u32;
      uint64_t* u64;
      int8_t* s8;
      int32_t* s32;
      int64_t* s64;
   };
}device_memory_t;

typedef struct
{
   struct
   {
      device_memory_t mem;
      VkImage image;
      VkSubresourceLayout mem_layout;
      VkImageLayout layout;
   }staging;
   device_memory_t mem;
   VkImage image;
   VkSubresourceLayout mem_layout;
   VkImageLayout layout;
   VkImageView view;
   VkSampler sampler;
   int width;
   int height;
   bool dirty;
}texture_t;

typedef struct
{
   device_memory_t mem;
   VkBuffer handle;
   VkDeviceSize size;
}buffer_t;

typedef struct
{
   VkDescriptorPool pool;
   VkDescriptorSet set;
   VkDescriptorSetLayout set_layout;
}descriptor_t;

typedef struct
{
   VkShaderModule vs;
   VkShaderModule fs;
}shaders_t;

typedef struct
{
   VkPipeline handle;
   VkPipelineLayout layout;
}pipeline_t;

void context_init(context_t* vk);
void context_free(context_t* vk);

typedef struct
{
   VkPhysicalDevice gpu;
   uint32_t queue_family_index;
   int width;
   int height;
}surface_init_info_t;
void surface_init(VkInstance instance, const surface_init_info_t* init_info, surface_t *surface);
void surface_free(VkInstance instance, surface_t *surface);

typedef struct
{
   VkSurfaceKHR surface;
   int width;
   int height;
   VkPresentModeKHR present_mode;
}swapchain_init_info_t;
void swapchain_init(VkDevice device, const swapchain_init_info_t* init_info, swapchain_t *chain);
void swapchain_free(const context_t *vk, swapchain_t *chain);

typedef struct
{
   int width;
   int height;
   const VkMemoryType* memory_types;
   uint32_t queue_family_index;
}texture_init_info_t;
void texture_init(VkDevice device, const texture_init_info_t *init_info, texture_t* tex);
void texture_free(const context_t* vk, texture_t* tex);
void texture_update(VkCommandBuffer cmd, texture_t* tex);

typedef struct
{
   uint32_t size;
   const void* data;
   const VkMemoryType* memory_types;
}vertex_buffer_init_info_t;
void vertex_buffer_init(VkDevice device, const vertex_buffer_init_info_t *init_info, buffer_t *vbo);
typedef struct
{
   uint32_t size;
   const VkMemoryType* memory_types;
}uniform_buffer_init_info_t;
void uniform_buffer_init(VkDevice device, const uniform_buffer_init_info_t* init_info, buffer_t *ubo);
void buffer_free(const context_t *vk, buffer_t *buffer);

void descriptors_init(const context_t* vk, const buffer_t* ubo, const texture_t* tex, descriptor_t* desc);
void descriptors_free(const context_t* vk, descriptor_t* desc);

void shaders_init(const context_t* vk, size_t vs_code_size, const uint32_t* vs_code, size_t fs_code_size, const uint32_t* fs_code, shaders_t *shaders);
void shaders_free(const context_t* vk, shaders_t *shaders);

void pipeline_init(const context_t* vk, const shaders_t *shaders,
                   int vertex_size, int attrib_count, const VkVertexInputAttributeDescription* attrib_desc,
                   const swapchain_t* chain, const descriptor_t* desc, pipeline_t* pipe);
void pipeline_free(const context_t* vk, pipeline_t* pipe);

void buffer_memory_init(VkDevice device, const VkMemoryType* memory_types, VkBuffer buffer, VkMemoryPropertyFlags req_flags, device_memory_t* mem);
void image_memory_init(VkDevice device, const VkMemoryType* memory_types, VkImage image, VkMemoryPropertyFlags req_flags, device_memory_t* mem);
void memory_free(const context_t* vk, device_memory_t* mem);
void memory_flush(VkDevice device, const device_memory_t* mem);
