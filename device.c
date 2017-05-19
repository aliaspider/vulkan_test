
#include <stdio.h>
#include <assert.h>
#include <vulkan/vulkan.h>

#include "vulkan.h"

#include <stdio.h>
#include <assert.h>
#include <vulkan/vulkan.h>

#include "vulkan.h"

void physical_device_init(VkInstance instance, physical_device_t* dst)
{
   {
      uint32_t one = 1;
      vkEnumeratePhysicalDevices(instance, &one, &dst->handle);
#if 0
      VkPhysicalDeviceProperties gpu_props;
      vkGetPhysicalDeviceProperties(vk->gpu, &gpu_props);
#endif
   }

   vkGetPhysicalDeviceMemoryProperties(dst->handle, &dst->mem);

#if 0
   {
      uint32_t deviceExtensionPropertiesCount;
      vkEnumerateDeviceExtensionProperties(vk->gpu, NULL, &deviceExtensionPropertiesCount, NULL);
      VkExtensionProperties pDeviceExtensionProperties[deviceExtensionPropertiesCount];
      vkEnumerateDeviceExtensionProperties(vk->gpu, NULL, &deviceExtensionPropertiesCount, pDeviceExtensionProperties);
   }
#endif
}

void physical_device_free(physical_device_t* gpu)
{
   gpu->handle = VK_NULL_HANDLE;
}

void device_init(VkPhysicalDevice gpu, device_t* dst)
{

   {
      uint32_t queueFamilyPropertyCount;
      vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertyCount, NULL);
      VkQueueFamilyProperties pQueueFamilyProperties[queueFamilyPropertyCount];
      vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertyCount, pQueueFamilyProperties);

      dst->queue_family_index = 0;

      int i;
      for (i = 0; i < queueFamilyPropertyCount; i++)
      {
         if ((pQueueFamilyProperties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)) ==
               (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT))
         {
            dst->queue_family_index = i;
            break;
         }
      }
   }
   {
      const float one = 1.0;
      const char *device_ext[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

      const VkDeviceQueueCreateInfo queue_info =
      {
         VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueFamilyIndex = dst->queue_family_index,
         .queueCount = 1,
         .pQueuePriorities = &one
      };

      const VkDeviceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
         .queueCreateInfoCount = 1,
         .pQueueCreateInfos = &queue_info,
         .enabledExtensionCount = countof(device_ext),
         .ppEnabledExtensionNames = device_ext,
      };
      vkCreateDevice(gpu, &info, NULL, &dst->handle);
   }

   /* get a device queue */
   vkGetDeviceQueue(dst->handle, dst->queue_family_index, 0, &dst->queue);

   /* create command buffer pool */
   {
      VkCommandPoolCreateInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
         .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         .queueFamilyIndex = dst->queue_family_index
      };
      vkCreateCommandPool(dst->handle, &info, NULL, &dst->cmd_pool);
   }

}

void device_free(device_t* device)
{
   vkDestroyCommandPool(device->handle, device->cmd_pool, NULL);
   vkDestroyDevice(device->handle, NULL);

   device->cmd_pool = VK_NULL_HANDLE;
   device->handle = VK_NULL_HANDLE;
}
