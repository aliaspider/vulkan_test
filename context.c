
#include <stdio.h>
#include <assert.h>
#include <vulkan/vulkan.h>

#include "vulkan.h"

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
   VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback)
{
   return VULKAN_CALL(vkCreateDebugReportCallbackEXT, instance, pCreateInfo, pAllocator, pCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,const VkAllocationCallbacks *pAllocator)
{
   return VULKAN_CALL(vkDestroyDebugReportCallbackEXT, instance, callback, pAllocator);
}

VkBool32 vulkan_debug_report_callback(VkDebugReportFlagsEXT flags,
                                      VkDebugReportObjectTypeEXT objectType,
                                      uint64_t object, size_t location,
                                      int32_t messageCode,
                                      const char *pLayerPrefix,
                                      const char *pMessage, void *pUserData)
{
   static const char* debugFlags_str[] = {"INFORMATION", "WARNING", "PERFORMANCE", "ERROR", "DEBUG"};

   int i;
   for (i = 0; i < countof(debugFlags_str); i++)
      if (flags & (1 << i))
         break;

   printf("[%-14s - %-11s] %s\n", pLayerPrefix, debugFlags_str[i],
          pMessage);

   assert((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) == 0);
   return VK_FALSE;
}


void context_init(context_t* vk)
{
   const char *layers[] =
   {
      "VK_LAYER_LUNARG_standard_validation",
      "VK_LAYER_GOOGLE_unique_objects",
      "VK_LAYER_LUNARG_swapchain",
      "VK_LAYER_LUNARG_core_validation",
      "VK_LAYER_LUNARG_image",
      "VK_LAYER_LUNARG_object_tracker",
      "VK_LAYER_LUNARG_parameter_validation",
      "VK_LAYER_GOOGLE_threading"
   };

   const char *instance_ext[] = {"VK_EXT_debug_report", "VK_KHR_surface",
                                 "VK_KHR_xlib_surface"
                                };
   uint32_t lprop_count;
   vkEnumerateInstanceLayerProperties(&lprop_count, NULL);
   VkLayerProperties lprops[lprop_count];
   vkEnumerateInstanceLayerProperties(&lprop_count, lprops);

   uint32_t iexprop_count;
   vkEnumerateInstanceExtensionProperties(layers[0], &iexprop_count, NULL);
   VkExtensionProperties iexprops[iexprop_count];
   vkEnumerateInstanceExtensionProperties(layers[0], &iexprop_count, iexprops);

   VkApplicationInfo appinfo =
   {
      VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Vulkan Test",
      .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
      .pEngineName = "my engine",
      .engineVersion = VK_MAKE_VERSION(0, 1, 0)
   };

   VkInstanceCreateInfo icinfo =
   {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appinfo,
      .enabledLayerCount = countof(layers),
      .ppEnabledLayerNames = layers,
      .enabledExtensionCount = countof(instance_ext),
      .ppEnabledExtensionNames = instance_ext
   };

   vkCreateInstance(&icinfo, NULL, &vk->instance);

   VkDebugReportCallbackCreateInfoEXT drccinfo =
   {
      VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
      .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
      VK_DEBUG_REPORT_WARNING_BIT_EXT |
      //      VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
      //      VK_DEBUG_REPORT_DEBUG_BIT_EXT |
      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
      .pfnCallback = vulkan_debug_report_callback,
      .pUserData = NULL
   };

   vkCreateDebugReportCallbackEXT(vk->instance, &drccinfo, NULL,
               &vk->debug_cb);

   uint32_t one = 1;
   vkEnumeratePhysicalDevices(vk->instance, &one, &vk->gpu);
   VkPhysicalDeviceProperties gpu_props;
   vkGetPhysicalDeviceProperties(vk->gpu, &gpu_props);

   uint32_t queueFamilyPropertyCount;
   vkGetPhysicalDeviceQueueFamilyProperties(vk->gpu, &queueFamilyPropertyCount, NULL);
   VkQueueFamilyProperties pQueueFamilyProperties[queueFamilyPropertyCount];
   vkGetPhysicalDeviceQueueFamilyProperties(vk->gpu, &queueFamilyPropertyCount, pQueueFamilyProperties);

   vk->queue_family_index = 0;

   int i;
   for (i = 0; i < queueFamilyPropertyCount; i++)
      if ((pQueueFamilyProperties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)) ==
            (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT))
      {
         vk->queue_family_index = i;
         break;
      }

   uint32_t deviceExtensionPropertiesCount;
   vkEnumerateDeviceExtensionProperties(vk->gpu, NULL, &deviceExtensionPropertiesCount, NULL);
   VkExtensionProperties pDeviceExtensionProperties[deviceExtensionPropertiesCount];
   vkEnumerateDeviceExtensionProperties(vk->gpu, NULL, &deviceExtensionPropertiesCount,
                                        pDeviceExtensionProperties);

   vkGetPhysicalDeviceMemoryProperties(vk->gpu, &vk->mem);

   /* init device */
   const char *device_ext[] = {"VK_KHR_swapchain"};

   const float one_f = 1.0;

   VkDeviceQueueCreateInfo queueCreateInfo =
   {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = vk->queue_family_index,
      .queueCount = 1,
      .pQueuePriorities = &one_f
   };

   VkDeviceCreateInfo dcinfo =
   {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledExtensionCount = countof(device_ext),
      .ppEnabledExtensionNames = device_ext,
   };
   vkCreateDevice(vk->gpu, &dcinfo, NULL, &vk->device);

   /* get a device queue */
   vkGetDeviceQueue(vk->device, vk->queue_family_index, 0, &vk->queue);

   VkCommandPoolCreateInfo commandPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
   commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                                 VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

   /* create command buffer pool */
   commandPoolCreateInfo.queueFamilyIndex = vk->queue_family_index;
   vkCreateCommandPool(vk->device, &commandPoolCreateInfo, NULL, &vk->cmd_pool);

}

void context_free(context_t* vk)
{
   vkDestroyCommandPool(vk->device, vk->cmd_pool, NULL);
   vkDestroyDevice(vk->device, NULL);
   vkDestroyDebugReportCallbackEXT(vk->instance, vk->debug_cb, NULL);
   vkDestroyInstance(vk->instance, NULL);

   vk->cmd_pool = VK_NULL_HANDLE;
   vk->device = VK_NULL_HANDLE;
   vk->debug_cb = VK_NULL_HANDLE;
   vk->instance = VK_NULL_HANDLE;
}
