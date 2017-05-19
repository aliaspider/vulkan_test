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
   VkPipeline handle;
   VkPipelineLayout layout;
}pipeline_t;

void context_init(context_t* dst);
void context_free(context_t* context);

typedef struct
{
   VkPhysicalDevice gpu;
   uint32_t queue_family_index;
   int width;
   int height;
}surface_init_info_t;
void surface_init(VkInstance instance, const surface_init_info_t* init_info, surface_t *dst);
void surface_free(VkInstance instance, surface_t *surface);

typedef struct
{
   VkSurfaceKHR surface;
   int width;
   int height;
   VkPresentModeKHR present_mode;
}swapchain_init_info_t;
void swapchain_init(VkDevice device, const swapchain_init_info_t* init_info, swapchain_t *dst);
void swapchain_free(VkDevice device, swapchain_t *chain);

typedef struct
{
   int width;
   int height;
   const VkMemoryType* memory_types;
   uint32_t queue_family_index;
}texture_init_info_t;
void texture_init(VkDevice device, const texture_init_info_t *init_info, texture_t* dst);
void texture_free(VkDevice device, texture_t* texture);
void texture_update(VkCommandBuffer cmd, texture_t* texture);

typedef struct
{
   const VkMemoryType* memory_types;
   VkBufferUsageFlags usage;
   uint32_t size;
   const void* data;
}buffer_init_info_t;
void buffer_init(VkDevice device, const buffer_init_info_t* init_info, buffer_t *dst);
void buffer_free(VkDevice device, buffer_t *buffer);

typedef struct
{
   VkBuffer ubo_buffer;
   VkDeviceSize ubo_range;
   VkSampler sampler;
   VkImageView image_view;
}descriptors_init_info_t;
void descriptors_init(VkDevice device, const descriptors_init_info_t* init_info, descriptor_t* dst);
void descriptors_free(VkDevice device, descriptor_t* descriptor);

typedef struct
{
   VkShaderModule vertex_shader;
   VkShaderModule fragment_shader;
   int vertex_size;
   int attrib_count;
   const VkVertexInputAttributeDescription* attrib_desc;
   VkDescriptorSetLayout set_layout;
   const VkRect2D* scissor;
   const VkViewport* viewport;
   VkRenderPass renderpass;
}pipeline_init_info_t;
void pipeline_init(VkDevice device, pipeline_init_info_t* init_info, pipeline_t* dst);
void pipeline_free(VkDevice device, pipeline_t* pipe);

typedef struct
{
   const VkMemoryType* memory_types;
   VkMemoryPropertyFlags req_flags;
   VkBuffer buffer;
   VkImage image;
}memory_init_info_t;
void memory_init(VkDevice device, const memory_init_info_t* init_info, device_memory_t* dst);
void memory_free(VkDevice device, device_memory_t* memory);
void memory_flush(VkDevice device, const device_memory_t* memory);
