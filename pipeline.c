
#include "vulkan.h"

void pipeline_init(const context_t* vk, const descriptor_t* desc, const swapchain_t* chain,
                   const shaders_t *shaders, pipeline_t* pipe)
{

   VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo[] =
   {
      {
         VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .pName = "main",
         .module = shaders->vs
      },
      {
         VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .pName = "main",
         .module = shaders->fs
      }
   };

   VkVertexInputBindingDescription vertexInputBindingDescription =
   {
      0, desc->vertex_input.size, VK_VERTEX_INPUT_RATE_VERTEX
   };

   VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo =
   {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1, &vertexInputBindingDescription,
      .vertexAttributeDescriptionCount = desc->vertex_input.count, desc->vertex_input.desc
   };

   VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo =
   {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
      .primitiveRestartEnable = VK_FALSE
   };

   VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo =
   {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1, &chain->viewport,
      .scissorCount = 1, &chain->scissor
   };

   VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo =
   {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//      .depthClampEnable = VK_FALSE,
//      .rasterizerDiscardEnable = VK_FALSE, /* ? */
//      .polygonMode = VK_POLYGON_MODE_FILL,
//      .cullMode = VK_CULL_MODE_NONE,
//      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
//      .depthBiasEnable = VK_FALSE,
//      .depthBiasConstantFactor = 0.0f,
//      .depthBiasClamp = 0.0f,
//      .depthBiasSlopeFactor = 0.0f,
      .lineWidth = 1.0f
   };

   VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo =
   {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
   };

   VkPipelineColorBlendAttachmentState colorBlendAttachmentState =
   {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
   };

   VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo =
   {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1, &colorBlendAttachmentState
   };

//   VkPushConstantRange pushConstantRanges[] =
//   {
//      {
//         .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
//         .offset = 0,
//         .size = sizeof(uniforms_t)
//      }
//   };

   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
   {
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1, &desc->set_layout,
//      .pushConstantRangeCount = countof(pushConstantRanges), pushConstantRanges
   };

   vkCreatePipelineLayout(vk->device, &pipelineLayoutCreateInfo, NULL, &pipe->layout);

   VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo =
   {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
      .stageCount = countof(pipelineShaderStageCreateInfo), pipelineShaderStageCreateInfo,
      .pVertexInputState = &pipelineVertexInputStateCreateInfo,
      .pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
      .pViewportState = &pipelineViewportStateCreateInfo,
      .pRasterizationState = &pipelineRasterizationStateCreateInfo,
      .pMultisampleState = &pipelineMultisampleStateCreateInfo,
      .pColorBlendState = &pipelineColorBlendStateCreateInfo,
      .layout = pipe->layout,
      .renderPass = chain->renderpass,
      .subpass = 0
   };

   vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, NULL, &pipe->handle);
}

void pipeline_free(const context_t *vk, pipeline_t *pipe)
{
   vkDestroyPipelineLayout(vk->device, pipe->layout, NULL);
   vkDestroyPipeline(vk->device, pipe->handle, NULL);
   pipe->layout = VK_NULL_HANDLE;
   pipe->handle = VK_NULL_HANDLE;
}
