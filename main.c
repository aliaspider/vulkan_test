
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <X11/Xutil.h>

#include "vulkan.h"
#include "png_file.h"

typedef struct
{
   struct
   {
      float x;
      float y;
   } center;
   struct
   {
      float width;
      float height;
   } image;
   struct
   {
      float width;
      float height;
   } screen;
   float angle;
} uniforms_t;

bool running = true;

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool handle_input(surface_t* surface, uniforms_t* uniforms)
{
   XEvent e;
   if (XCheckWindowEvent(surface->display, surface->window, ~0, &e) && (e.type == KeyPress))
   {
      if(e.xkey.state & Mod1Mask)
      {
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Left))
            uniforms->angle += M_PI / 64;
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Right))
            uniforms->angle -= M_PI / 64;
      }
      else if(e.xkey.state & ControlMask)
      {
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Left))
            uniforms->image.width += 16.0f;
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Right))
            uniforms->image.width -= 16.0f;
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Up))
            uniforms->image.height += 16.0f;
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Down))
            uniforms->image.height -= 16.0f;
      }
      else
      {
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_q))
         {
            running = false;
            return false;
         }
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Left))
            uniforms->center.x -= 16.0f;
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Right))
            uniforms->center.x += 16.0f;
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Up))
            uniforms->center.y -= 16.0f;
         if(e.xkey.keycode == XKeysymToKeycode(surface->display, XK_Down))
            uniforms->center.y += 16.0f;
      }
      return true;
   }
   return false;
}
#endif

int main(int argc, char **argv)
{
   printf("Vulkan test:\n");

   instance_t instance;
   instance_init(&instance);

   physical_device_t gpu;
   physical_device_init(instance.handle, &gpu);

   device_t device;
   device_init(gpu.handle, &device);

   surface_t surface;
   {
      surface_init_info_t info =
      {
         .gpu = gpu.handle,
         .queue_family_index = device.queue_family_index,
         .width = 640,
         .height = 480
      };
      surface_init(instance.handle, &info, &surface);
   }

   swapchain_t chain;
   {
      swapchain_init_info_t info =
      {
         .surface = surface.handle,
         .width = surface.width,
         .height = surface.height,
         .present_mode = VK_PRESENT_MODE_FIFO_KHR
//         .present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR
      };
      swapchain_init(device.handle, &info, &chain);
   }

   texture_t tex;
   {
      png_file_t png;
      png_file_init("texture.png", &png);

      {
         texture_init_info_t info =
         {
            .queue_family_index = device.queue_family_index,
            .width = png.width,
            .height = png.height,
         };
         texture_init(device.handle, gpu.mem.memoryTypes, &info, &tex);
      }

      /* texture updates are written to the stating texture then uploaded later */
      png_file_read(&png, tex.staging.mem.u8 + tex.staging.mem_layout.offset, tex.staging.mem_layout.rowPitch);
      device_memory_flush(device.handle, &tex.staging.mem);
      tex.dirty = true;

      png_file_free(&png);
   }

   typedef struct
   {
      struct
      {
         float x, y, z, w;
      } position;
      struct
      {
         float u, v;
      } texcoord;
      struct
      {
         float r, g, b, a;
      } color;
   } vertex_t;

   buffer_t vbo;
   {
      const vertex_t vertices[] =
      {
         {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
         {{ 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
         {{ 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
         {{-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
      };

      buffer_init_info_t info =
      {
         .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         .size = sizeof(vertices),
         .data = vertices,
      };
      buffer_init(device.handle, gpu.mem.memoryTypes, &info, &vbo);
   }

   buffer_t  ubo;
   {
      buffer_init_info_t info =
      {
         .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         .size = sizeof(uniforms_t),
      };
      buffer_init(device.handle, gpu.mem.memoryTypes, &info, &ubo);
   }

   descriptor_t desc;
   {
      descriptors_init_info_t info =
      {
         .ubo_buffer = ubo.handle,
         .ubo_range = ubo.size,
         .sampler = tex.sampler,
         .image_view = tex.view,
      };
      descriptors_init(device.handle, &info, &desc);
   }

   pipeline_t pipe;
   {
      VkShaderModule vertex_shader;
      {
         static const uint32_t code [] =
         #include "main.vert.inc"
         ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(device.handle, &info, NULL, &vertex_shader);
      }
      
      VkShaderModule fragment_shader;
      {
         static const uint32_t code [] =
         #include "main.frag.inc"
         ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(device.handle, &info, NULL, &fragment_shader);
      }
      
      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, position)},
         {1, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(vertex_t, texcoord)},
         {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, color)}
      };

      {
         pipeline_init_info_t info =
         {
            .vertex_shader = vertex_shader,
            .fragment_shader = fragment_shader,
            .vertex_size = sizeof(vertex_t),
            .attrib_count = countof(attrib_desc),
            .attrib_desc = attrib_desc,
            .set_layout = desc.set_layout,
            .scissor = &chain.scissor,
            .viewport = &chain.viewport,
            .renderpass = chain.renderpass,
         };
         pipeline_init(device.handle, &info, &pipe);
      }

      vkDestroyShaderModule(device.handle, vertex_shader, NULL);
      vkDestroyShaderModule(device.handle, fragment_shader, NULL);
   }

   VkCommandBuffer cmd;
   {
      const VkCommandBufferAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .commandPool = device.cmd_pool,
         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = 1
      };
      vkAllocateCommandBuffers(device.handle, &info, &cmd);
   }

   VkFence queue_fence;
   VkFence chain_fence;
   {
      VkFenceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
         .flags = VK_FENCE_CREATE_SIGNALED_BIT
      };
      vkCreateFence(device.handle, &info, NULL, &chain_fence);
      vkCreateFence(device.handle, &info, NULL, &queue_fence);
   }

   int frames = 0;
   struct timespec start_time;
   clock_gettime(CLOCK_MONOTONIC, &start_time);

   uniforms_t *uniforms = ubo.mem.ptr;
   uniforms->center.x = 0.0f;
   uniforms->center.y = 0.0f;
   uniforms->image.width  = tex.width;
   uniforms->image.height = tex.height;
   uniforms->screen.width  = chain.viewport.width;
   uniforms->screen.height = chain.viewport.height;
   uniforms->angle = 0.0f;

   while (running)
   {
      uint32_t image_index;
      vkWaitForFences(device.handle, 1, &chain_fence, VK_TRUE, -1);
      vkResetFences(device.handle, 1, &chain_fence);
      vkAcquireNextImageKHR(device.handle, chain.handle, UINT64_MAX, NULL, chain_fence, &image_index);

      vkWaitForFences(device.handle, 1, &queue_fence, VK_TRUE, -1);
      vkResetFences(device.handle, 1, &queue_fence);

      {
         const VkCommandBufferBeginInfo info =
         {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
         };
         vkBeginCommandBuffer(cmd, &info);
      }

      if(tex.dirty)
         texture_update(cmd, &tex);

      /* renderpass */
      {
         {
            const VkClearValue clearValue = {{{0.0f, 0.1f, 1.0f, 0.0f}}};
            const VkRenderPassBeginInfo info =
            {
               VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
               .renderPass = chain.renderpass,
               .framebuffer = chain.framebuffers[image_index],
               .renderArea = chain.scissor,
               .clearValueCount = 1,
               .pClearValues = &clearValue
            };
            vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
         }

         vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle);
         vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.layout, 0, 1, &desc.set, 0 , NULL);
//         vkCmdPushConstants(device.cmd, device.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uniforms_t), mapped_uniforms);

         VkDeviceSize offset = 0;
         vkCmdBindVertexBuffers(cmd, 0, 1, &vbo.handle, &offset);

         vkCmdDraw(cmd, vbo.size / sizeof(vertex_t), 1, 0, 0);

         vkCmdEndRenderPass(cmd);
      }

      vkEndCommandBuffer(cmd);

      {
         const VkSubmitInfo info =
         {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd
         };
         vkQueueSubmit(device.queue, 1, &info, queue_fence);
      }

      {
         const VkPresentInfoKHR info =
         {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = &chain.handle,
            .pImageIndices = &image_index
         };
         vkQueuePresentKHR(device.queue, &info);
      }

      struct timespec end_time;
      clock_gettime(CLOCK_MONOTONIC, &end_time);
      float diff = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0f;
      frames++;

      if(handle_input(&surface, uniforms))
         device_memory_flush(device.handle, &ubo.mem);

      if (diff > 0.5f)
      {
         printf("\r fps: %f", frames / diff);
         frames = 0;
         start_time = end_time;
         fflush(stdout);
      }
   }   
   printf("\n");

   vkWaitForFences(device.handle, 1, &queue_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(device.handle, queue_fence, NULL);

   vkWaitForFences(device.handle, 1, &chain_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(device.handle, chain_fence, NULL);

   pipeline_free(device.handle, &pipe);
   descriptors_free(device.handle, &desc);
   buffer_free(device.handle, &ubo);
   buffer_free(device.handle, &vbo);
   texture_free(device.handle, &tex);
   swapchain_free(device.handle, &chain);
   surface_free(instance.handle, &surface);
   device_free(&device);
   physical_device_free(&gpu);
   instance_free(&instance);

   return 0;
}
