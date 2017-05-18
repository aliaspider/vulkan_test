
#include "vulkan.h"

void surface_init(VkInstance instance, const surface_init_info_t* init_info, surface_t *surface)
{
   /* init display and window */
   #ifdef VK_USE_PLATFORM_XLIB_KHR
   XInitThreads();
   surface->display = XOpenDisplay(NULL);
   surface->window  = XCreateSimpleWindow(surface->display, DefaultRootWindow(surface->display), 0, 0, init_info->width, init_info->height, 0, 0, 0);
   XStoreName(surface->display, surface->window, "Vulkan Test");
   XSelectInput(surface->display, surface->window, ExposureMask | KeyPressMask);
   XMapWindow(surface->display, surface->window);

   /* init surface */
   VkXlibSurfaceCreateInfoKHR xlibSurfaceCreateInfo =
   {
      VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
      .dpy = surface->display,
      .window = surface->window
   };
   vkCreateXlibSurfaceKHR(instance, &xlibSurfaceCreateInfo, NULL, &surface->handle);
   #else
   #error platform not supported
   #endif

   /* init swapchain */
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(init_info->gpu, surface->handle, &surfaceCapabilities);

   VkBool32 physicalDeviceSurfaceSupport;
   vkGetPhysicalDeviceSurfaceSupportKHR(init_info->gpu, init_info->queue_family_index, surface->handle, &physicalDeviceSurfaceSupport);

   uint32_t surfaceFormatcount;
   vkGetPhysicalDeviceSurfaceFormatsKHR(init_info->gpu, surface->handle, &surfaceFormatcount, NULL);
   VkSurfaceFormatKHR surfaceFormats[surfaceFormatcount];
   vkGetPhysicalDeviceSurfaceFormatsKHR(init_info->gpu, surface->handle, &surfaceFormatcount, surfaceFormats);

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR(init_info->gpu, surface->handle, &presentModeCount, NULL);
   VkPresentModeKHR presentModes[presentModeCount];
   vkGetPhysicalDeviceSurfacePresentModesKHR(init_info->gpu, surface->handle, &presentModeCount, presentModes);

   surface->width = init_info->width;
   surface->height = init_info->height;
}

void surface_free(VkInstance instance, surface_t *surface)
{
   vkDestroySurfaceKHR(instance, surface->handle, NULL);
   XDestroyWindow(surface->display, surface->window);
   XCloseDisplay(surface->display);

   surface->handle = VK_NULL_HANDLE;
   surface->display = NULL;

}
