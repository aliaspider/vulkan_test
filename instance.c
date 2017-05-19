
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


void instance_init(instance_t* dst)
{
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
      const char *instance_ext[] =
      {
         VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
         VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_XLIB_KHR
         VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
      };
#if 0
      uint32_t lprop_count;
      vkEnumerateInstanceLayerProperties(&lprop_count, NULL);
      VkLayerProperties lprops[lprop_count];
      vkEnumerateInstanceLayerProperties(&lprop_count, lprops);

      uint32_t iexprop_count;
      vkEnumerateInstanceExtensionProperties(layers[0], &iexprop_count, NULL);
      VkExtensionProperties iexprops[iexprop_count];
      vkEnumerateInstanceExtensionProperties(layers[0], &iexprop_count, iexprops);
#endif
      VkApplicationInfo appinfo =
      {
         VK_STRUCTURE_TYPE_APPLICATION_INFO,
         .pApplicationName = "Vulkan Test",
         .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
         .pEngineName = "my engine",
         .engineVersion = VK_MAKE_VERSION(0, 1, 0)
      };

      VkInstanceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
         .pApplicationInfo = &appinfo,
         .enabledLayerCount = countof(layers),
         .ppEnabledLayerNames = layers,
         .enabledExtensionCount = countof(instance_ext),
         .ppEnabledExtensionNames = instance_ext
      };
      vkCreateInstance(&info, NULL, &dst->handle);
   }

   {
      VkDebugReportCallbackCreateInfoEXT info =
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
      vkCreateDebugReportCallbackEXT(dst->handle, &info, NULL, &dst->debug_cb);
   }

}

void instance_free(instance_t* instance)
{
   vkDestroyDebugReportCallbackEXT(instance->handle, instance->debug_cb, NULL);
   vkDestroyInstance(instance->handle, NULL);

   instance->debug_cb = VK_NULL_HANDLE;
   instance->handle = VK_NULL_HANDLE;
}
