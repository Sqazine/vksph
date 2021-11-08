#include "App.h"
#include <iostream>

App::App(std::string title, int32_t width, int32_t height)
	: m_IsRunning(true), m_WindowWidth(width), m_WindowHeight(height), m_WindowTitle(title)
{
}

App::~App()
{
	m_Device->WaitIdle();

	m_DeletionQueue.Flush();

	SDL_Quit();
}

void App::Run()
{
	Init();
	while (m_IsRunning)
	{
		Update();
		Simulate();
		Draw();
		SubmitAndPresent();
	}
}

void App::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		SDL_Log("failed to init SDL!");

#if _DEBUG
	std::string str = "glslangValidator -V " + std::string(SHADER_DIR) + "particle.vert -o particle.vert.spv";
	system(str.c_str());
	str = "glslangValidator -V " + std::string(SHADER_DIR) + "particle.frag -o particle.frag.spv";
	system(str.c_str());
	str = "glslangValidator -V " + std::string(SHADER_DIR) + "density_pressure.comp -o density_pressure.comp.spv";
	system(str.c_str());
	str = "glslangValidator -V " + std::string(SHADER_DIR) + "force.comp -o force.comp.spv";
	system(str.c_str());
	str = "glslangValidator -V " + std::string(SHADER_DIR) + "integrate.comp -o integrate.comp.spv";
	system(str.c_str());
#endif

	CreateWindow(m_WindowTitle, m_WindowWidth, m_WindowHeight);
	LoadVulkanLib();

	m_Instance = std::make_unique<VulkanInstance>(m_WindowHandle, validationLayers);
	m_Device = std::make_unique<VulkanDevice>(m_Instance.get(), deviceExtensions);
	m_SwapChain = std::make_unique<VulkanSwapChain>(m_WindowHandle, m_Instance.get(), m_Device.get());
	m_RenderPass = std::make_unique<VulkanRenderPass>(m_Device.get(), m_SwapChain.get());

	for (size_t i = 0; i < m_SwapChain->GetVKSwapChainImageViews().size(); ++i)
		{
			std::vector<VkImageView> views={m_SwapChain->GetVKSwapChainImageViews()[i]};
			m_SwapChainFrameBuffers.emplace_back(std::make_unique<VulkanFramebuffer>(m_Device->GetLogicalDeviceHandle(),
																				 m_RenderPass->GetVKRenderPassHandle(),
																				 views,
																				 m_SwapChain->GetVKSwapChainExtent().width,
																				 m_SwapChain->GetVKSwapChainExtent().height));
		}

	CreateDescriptorPool();
	CreatePipelineCache();
	CreateGraphicsPipelineLayout();
	CreateGraphicsPipeline();
	CreateGraphicsCommandPool();
	CreateGraphicsCommandBuffers();
	CreateSemaphores();
	CreateComputeDescriptorSetLayout();
	CreateComputePipelineLayout();
	CreateComputePipelines();
	CreateComputeCommandPool();
	CreateComputeCommandBuffer();

	std::array<glm::vec2, PARTICLE_NUM> initParticlePosition;
	for (auto i = 0, x = 0, y = 0; i < PARTICLE_NUM; ++i)
	{
		initParticlePosition[i].x = -0.625f + PARTICLE_RADIUS * 2 * x;
		initParticlePosition[i].y = -1 + PARTICLE_RADIUS * 2 * y;
		x++;
		if (x >= 125)
		{
			x = 0;
			y++;
		}
	}
	CreatePackedParticleBuffer();
	InitParticleData(initParticlePosition);
	UpdateComputeDescriptorSets();
}

void App::Update()
{
	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			m_IsRunning = false;
		}
	}
	const uint8_t *keyboardState = SDL_GetKeyboardState(nullptr);
	if (keyboardState[SDL_SCANCODE_ESCAPE])
		m_IsRunning = false;

	if (keyboardState[SDL_SCANCODE_1])
	{
		std::array<glm::vec2, PARTICLE_NUM> initParticlePosition;
		for (auto i = 0, x = 0, y = 0; i < PARTICLE_NUM; ++i)
		{
			initParticlePosition[i].x = -0.625f + PARTICLE_RADIUS * 2 * x;
			initParticlePosition[i].y = -1 + PARTICLE_RADIUS * 2 * y;
			x++;
			if (x >= 125)
			{
				x = 0;
				y++;
			}
		}
		InitParticleData(initParticlePosition);
	}
	if (keyboardState[SDL_SCANCODE_2])
	{
		std::array<glm::vec2, PARTICLE_NUM> initParticlePosition;
		for (auto i = 0, x = 0, y = 0; i < PARTICLE_NUM; ++i)
		{
			initParticlePosition[i].x = -1 + PARTICLE_RADIUS * 2 * x;
			initParticlePosition[i].y = 1 - PARTICLE_RADIUS * 2 * y;
			x++;
			if (x >= 100)
			{
				x = 0;
				y++;
			}
		}
		InitParticleData(initParticlePosition);
	}
	if (keyboardState[SDL_SCANCODE_3])
	{
		std::array<glm::vec2, PARTICLE_NUM> initParticlePosition;
		for (auto i = 0, x = 0, y = 0; i < PARTICLE_NUM; ++i)
		{
			initParticlePosition[i].x = 1 - PARTICLE_RADIUS * 2 * x;
			initParticlePosition[i].y = -1 + PARTICLE_RADIUS * 2 * y;
			x++;
			if (x >= 100)
			{
				x = 0;
				y++;
			}
		}
		InitParticleData(initParticlePosition);
	}

	VkSubmitInfo computeSubmitInfo{};
	computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	computeSubmitInfo.pNext = nullptr;
	computeSubmitInfo.waitSemaphoreCount = 0;
	computeSubmitInfo.pWaitSemaphores = nullptr;
	computeSubmitInfo.pWaitDstStageMask = 0;
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &m_ComputeCommandBufferHandle;
	computeSubmitInfo.signalSemaphoreCount = 0;
	computeSubmitInfo.pSignalSemaphores = nullptr;

	//update particle
	VK_CHECK(m_Device->GetComputeQueue()->Submit(1, &computeSubmitInfo, VK_NULL_HANDLE));
}
void App::Simulate()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(m_ComputeCommandBufferHandle, &beginInfo));

	vkCmdBindDescriptorSets(m_ComputeCommandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineLayoutHandle, 0, 1, &m_ComputeDescriptorSetHandle, 0, nullptr);

	vkCmdBindPipeline(m_ComputeCommandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineHandles[0]);
	vkCmdDispatch(m_ComputeCommandBufferHandle, WORK_GROUP_NUM, 1, 1);
	vkCmdPipelineBarrier(m_ComputeCommandBufferHandle, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	vkCmdBindPipeline(m_ComputeCommandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineHandles[1]);
	vkCmdDispatch(m_ComputeCommandBufferHandle, WORK_GROUP_NUM, 1, 1);
	vkCmdPipelineBarrier(m_ComputeCommandBufferHandle, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	vkCmdBindPipeline(m_ComputeCommandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineHandles[2]);
	vkCmdDispatch(m_ComputeCommandBufferHandle, WORK_GROUP_NUM, 1, 1);
	vkCmdPipelineBarrier(m_ComputeCommandBufferHandle, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	VK_CHECK(vkEndCommandBuffer(m_ComputeCommandBufferHandle));
}

void App::Draw()
{
	for (size_t i = 0; i < m_GraphicsCommandBufferHandles.size(); ++i)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(m_GraphicsCommandBufferHandles[i], &commandBufferBeginInfo);

		VkClearValue clearValue{0.0f, 0.0f, 0.0f, 1.0f};
		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_RenderPass->GetVKRenderPassHandle();
		renderPassBeginInfo.framebuffer = m_SwapChainFrameBuffers[i]->GetVKFramebufferHandle();
		renderPassBeginInfo.renderArea.offset = {0, 0};
		renderPassBeginInfo.renderArea.extent = m_SwapChain->GetVKSwapChainExtent();
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(m_GraphicsCommandBufferHandles[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewPort;
		viewPort.x = 0;
		viewPort.y = 0;
		viewPort.width = m_SwapChain->GetVKSwapChainExtent().width;
		viewPort.height = m_SwapChain->GetVKSwapChainExtent().height;
		viewPort.minDepth = 0;
		viewPort.maxDepth = 1;

		VkRect2D scissor;
		scissor.extent = m_SwapChain->GetVKSwapChainExtent();
		scissor.offset = {0, 0};

		vkCmdSetViewport(m_GraphicsCommandBufferHandles[i], 0, 1, &viewPort);
		vkCmdBindPipeline(m_GraphicsCommandBufferHandles[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicePipelineHandle);

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(m_GraphicsCommandBufferHandles[i], 0, 1, &m_PackedParticlesBufferHandle, &offsets);
		vkCmdDraw(m_GraphicsCommandBufferHandles[i], PARTICLE_NUM, 1, 0, 0);
		vkCmdEndRenderPass(m_GraphicsCommandBufferHandles[i]);

		VK_CHECK(vkEndCommandBuffer(m_GraphicsCommandBufferHandles[i]));
	}
}

void App::SubmitAndPresent()
{
	vkAcquireNextImageKHR(m_Device->GetLogicalDeviceHandle(), m_SwapChain->GetVKSwapChainHandle(), UINT64_MAX, m_ImageAvailableSemaphoreHandle, VK_NULL_HANDLE, &m_SwapChainImageIndex);

	VkSubmitInfo graphicsSubmitInfo{};
	graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	graphicsSubmitInfo.pNext = nullptr;
	graphicsSubmitInfo.waitSemaphoreCount = 1;
	graphicsSubmitInfo.pWaitSemaphores = &m_ImageAvailableSemaphoreHandle;
	graphicsSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
	graphicsSubmitInfo.commandBufferCount = 1;
	graphicsSubmitInfo.pCommandBuffers = m_GraphicsCommandBufferHandles.data() + m_SwapChainImageIndex;
	graphicsSubmitInfo.signalSemaphoreCount = 1;
	graphicsSubmitInfo.pSignalSemaphores = &m_RenderFinishedSemaphoreHandle;

	VK_CHECK(m_Device->GetGraphicsQueue()->Submit(1, &graphicsSubmitInfo, VK_NULL_HANDLE));

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphoreHandle;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_SwapChain->GetVKSwapChainHandle();
	presentInfo.pImageIndices = &m_SwapChainImageIndex;
	presentInfo.pResults = nullptr;

	m_Device->GetPresentQueue()->Present(&presentInfo);
	m_Device->GetPresentQueue()->WaitIdle();
}

void App::CreateWindow(std::string title, int32_t width, int32_t height)
{
	uint32_t windowFlag = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;

	m_WindowHandle = SDL_CreateWindow(title.c_str(),
									  SDL_WINDOWPOS_CENTERED,
									  SDL_WINDOWPOS_CENTERED,
									  width,
									  height,
									  windowFlag);

	m_DeletionQueue.Add([=]()
						{ SDL_DestroyWindow(m_WindowHandle); });
}

void App::LoadVulkanLib()
{

	int flag = SDL_Vulkan_LoadLibrary(nullptr);
	if (flag != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Not Support vulkan:%s\n", SDL_GetError());
		exit(1);
	}

	void *instanceProcAddr = SDL_Vulkan_GetVkGetInstanceProcAddr();
	if (!instanceProcAddr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "SDL_Vulkan_GetVkGetInstanceProcAddr(): %s\n",
					 SDL_GetError());
		exit(1);
	}

	m_DeletionQueue.Add([=]()
						{ SDL_Vulkan_UnloadLibrary(); });
}

void App::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSize.descriptorCount = 5;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.maxSets = 1;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	VK_CHECK(vkCreateDescriptorPool(m_Device->GetLogicalDeviceHandle(), &poolInfo, nullptr, &m_GlobalDescriptorPoolHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyDescriptorPool(m_Device->GetLogicalDeviceHandle(), m_GlobalDescriptorPoolHandle, nullptr); });
}

void App::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.initialDataSize = 0;
	info.pInitialData = nullptr;

	VK_CHECK(vkCreatePipelineCache(m_Device->GetLogicalDeviceHandle(), &info, nullptr, &m_GlobalPipelineCacheHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyPipelineCache(m_Device->GetLogicalDeviceHandle(), m_GlobalPipelineCacheHandle, nullptr); });
}

void App::CreatePackedParticleBuffer()
{
	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.flags = 0;
	bufferInfo.size = m_PackedBufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(m_Device->GetLogicalDeviceHandle(), &bufferInfo, nullptr, &m_PackedParticlesBufferHandle));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_Device->GetLogicalDeviceHandle(), m_PackedParticlesBufferHandle, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = m_Device->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vkAllocateMemory(m_Device->GetLogicalDeviceHandle(), &allocInfo, nullptr, &m_PackedParticleBufferMemoryHandle));

	vkBindBufferMemory(m_Device->GetLogicalDeviceHandle(), m_PackedParticlesBufferHandle, m_PackedParticleBufferMemoryHandle, 0);

	m_DeletionQueue.Add([=]()
						{
							vkDestroyBuffer(m_Device->GetLogicalDeviceHandle(), m_PackedParticlesBufferHandle, nullptr);
							vkFreeMemory(m_Device->GetLogicalDeviceHandle(), m_PackedParticleBufferMemoryHandle, nullptr);
						});
}

void App::CreateGraphicsPipelineLayout()
{
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.setLayoutCount = 0;
	info.pSetLayouts = nullptr;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;

	VK_CHECK(vkCreatePipelineLayout(m_Device->GetLogicalDeviceHandle(), &info, nullptr, &m_GraphicsPipelineLayoutHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyPipelineLayout(m_Device->GetLogicalDeviceHandle(), m_GraphicsPipelineLayoutHandle, nullptr); });
}

void App::CreateGraphicsPipeline()
{
	VkShaderModule vertShaderModule = CreateShaderModuleFromSpirvFile(m_Device->GetLogicalDeviceHandle(), "particle.vert.spv");
	VkShaderModule fragShaderModule = CreateShaderModuleFromSpirvFile(m_Device->GetLogicalDeviceHandle(), "particle.frag.spv");

	VkPipelineShaderStageCreateInfo vertStageInfo = {};
	vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageInfo.pNext = nullptr;
	vertStageInfo.flags = 0;
	vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStageInfo.module = vertShaderModule;
	vertStageInfo.pName = "main";
	vertStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragStageInfo = {};
	fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageInfo.pNext = nullptr;
	fragStageInfo.flags = 0;
	fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageInfo.module = fragShaderModule;
	fragStageInfo.pName = "main";
	fragStageInfo.pSpecializationInfo = nullptr;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
	shaderStageCreateInfos.emplace_back(vertStageInfo);
	shaderStageCreateInfos.emplace_back(fragStageInfo);

	VkVertexInputBindingDescription vertexInputBIndingDesc = {};
	vertexInputBIndingDesc.binding = 0;
	vertexInputBIndingDesc.stride = sizeof(glm::vec2);
	vertexInputBIndingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexInputAttribDecs = {};
	vertexInputAttribDecs.binding = 0;
	vertexInputAttribDecs.location = 0;
	vertexInputAttribDecs.offset = 0;
	vertexInputAttribDecs.format = VK_FORMAT_R32G32_SFLOAT;

	VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
	vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateInfo.pNext = nullptr;
	vertexInputStateInfo.flags = 0;
	vertexInputStateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBIndingDesc;
	vertexInputStateInfo.vertexAttributeDescriptionCount = 1;
	vertexInputStateInfo.pVertexAttributeDescriptions = &vertexInputAttribDecs;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
	inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateInfo.pNext = nullptr;
	inputAssemblyStateInfo.flags = 0;
	inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewPort = {};
	viewPort.x = 0;
	viewPort.y = 0;
	viewPort.width = (float)m_SwapChain->GetVKSwapChainExtent().width;
	viewPort.height = (float)m_SwapChain->GetVKSwapChainExtent().height;
	viewPort.minDepth = 0;
	viewPort.maxDepth = 1;

	VkRect2D scissor = {};
	scissor.extent = m_SwapChain->GetVKSwapChainExtent();
	scissor.offset = {0, 0};

	VkPipelineViewportStateCreateInfo viewportStateInfo = {};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.pNext = nullptr;
	viewportStateInfo.flags = 0;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewPort;

	VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
	rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateInfo.pNext = nullptr;
	rasterizationStateInfo.flags = 0;
	rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateInfo.lineWidth = 1;
	rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateInfo.depthBiasClamp = 0;
	rasterizationStateInfo.depthBiasConstantFactor = 0;
	rasterizationStateInfo.depthBiasSlopeFactor = 0;
	rasterizationStateInfo.cullMode = VK_CULL_MODE_NONE;

	VkPipelineMultisampleStateCreateInfo multiSampleStateInfo;
	multiSampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampleStateInfo.pNext = nullptr;
	multiSampleStateInfo.flags = 0;
	multiSampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampleStateInfo.alphaToCoverageEnable = VK_FALSE;
	multiSampleStateInfo.alphaToOneEnable = VK_FALSE;
	multiSampleStateInfo.minSampleShading = 0;
	multiSampleStateInfo.pSampleMask = nullptr;
	multiSampleStateInfo.sampleShadingEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
	colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateInfo.pNext = nullptr;
	colorBlendStateInfo.flags = 0;
	colorBlendStateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateInfo.attachmentCount = 1;
	colorBlendStateInfo.pAttachments = &colorBlendAttachment;
	colorBlendStateInfo.blendConstants[0] = 0.0f;
	colorBlendStateInfo.blendConstants[1] = 0.0f;
	colorBlendStateInfo.blendConstants[2] = 0.0f;
	colorBlendStateInfo.blendConstants[3] = 0.0f;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.pNext = nullptr;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.stageCount = shaderStageCreateInfos.size();
	graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos.data();
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateInfo;
	graphicsPipelineCreateInfo.pTessellationState = nullptr;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multiSampleStateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateInfo;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;
	graphicsPipelineCreateInfo.layout = m_GraphicsPipelineLayoutHandle;
	graphicsPipelineCreateInfo.renderPass = m_RenderPass->GetVKRenderPassHandle();
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	VK_CHECK(vkCreateGraphicsPipelines(m_Device->GetLogicalDeviceHandle(), m_GlobalPipelineCacheHandle, 1, &graphicsPipelineCreateInfo, nullptr, &m_GraphicePipelineHandle));

	vkDestroyShaderModule(m_Device->GetLogicalDeviceHandle(), vertShaderModule, nullptr);
	vkDestroyShaderModule(m_Device->GetLogicalDeviceHandle(), fragShaderModule, nullptr);

	m_DeletionQueue.Add([=]()
						{ vkDestroyPipeline(m_Device->GetLogicalDeviceHandle(), m_GraphicePipelineHandle, nullptr); });
}

void App::CreateGraphicsCommandPool()
{
	VkCommandPoolCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.queueFamilyIndex = m_Device->GetQueueIndices().graphicsFamily.value();

	VK_CHECK(vkCreateCommandPool(m_Device->GetLogicalDeviceHandle(), &info, nullptr, &m_GraphicsCommandPoolHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyCommandPool(m_Device->GetLogicalDeviceHandle(), m_GraphicsCommandPoolHandle, nullptr); });
}

void App::CreateGraphicsCommandBuffers()
{
	m_GraphicsCommandBufferHandles.resize(m_SwapChainFrameBuffers.size());
	VkCommandBufferAllocateInfo bufferAllocInfo = {};
	bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocInfo.pNext = nullptr;
	bufferAllocInfo.commandBufferCount = (uint32_t)m_GraphicsCommandBufferHandles.size();
	bufferAllocInfo.commandPool = m_GraphicsCommandPoolHandle;
	bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VK_CHECK(vkAllocateCommandBuffers(m_Device->GetLogicalDeviceHandle(), &bufferAllocInfo, m_GraphicsCommandBufferHandles.data()));

	m_DeletionQueue.Add([=]()
						{ vkFreeCommandBuffers(m_Device->GetLogicalDeviceHandle(), m_GraphicsCommandPoolHandle, m_GraphicsCommandBufferHandles.size(), m_GraphicsCommandBufferHandles.data()); });
}

void App::CreateSemaphores()
{
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;

	VK_CHECK(vkCreateSemaphore(m_Device->GetLogicalDeviceHandle(), &info, nullptr, &m_ImageAvailableSemaphoreHandle));
	VK_CHECK(vkCreateSemaphore(m_Device->GetLogicalDeviceHandle(), &info, nullptr, &m_RenderFinishedSemaphoreHandle));

	m_DeletionQueue.Add([=]()
						{
							vkDestroySemaphore(m_Device->GetLogicalDeviceHandle(), m_RenderFinishedSemaphoreHandle, nullptr);
							vkDestroySemaphore(m_Device->GetLogicalDeviceHandle(), m_ImageAvailableSemaphoreHandle, nullptr);
						});
}

void App::CreateComputeDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[5];

	descriptorSetLayoutBindings[0].binding = 0;
	descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBindings[0].descriptorCount = 1;
	descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;

	descriptorSetLayoutBindings[1].binding = 1;
	descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBindings[1].descriptorCount = 1;
	descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;

	descriptorSetLayoutBindings[2].binding = 2;
	descriptorSetLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBindings[2].descriptorCount = 1;
	descriptorSetLayoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBindings[2].pImmutableSamplers = nullptr;

	descriptorSetLayoutBindings[3].binding = 3;
	descriptorSetLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBindings[3].descriptorCount = 1;
	descriptorSetLayoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBindings[3].pImmutableSamplers = nullptr;

	descriptorSetLayoutBindings[4].binding = 4;
	descriptorSetLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBindings[4].descriptorCount = 1;
	descriptorSetLayoutBindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBindings[4].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.pNext = nullptr;
	descriptorSetLayoutInfo.flags = 0;
	descriptorSetLayoutInfo.bindingCount = 5;
	descriptorSetLayoutInfo.pBindings = descriptorSetLayoutBindings;

	VK_CHECK(vkCreateDescriptorSetLayout(m_Device->GetLogicalDeviceHandle(), &descriptorSetLayoutInfo, nullptr, &m_ComputeDescriptorSetLayoutHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyDescriptorSetLayout(m_Device->GetLogicalDeviceHandle(), m_ComputeDescriptorSetLayoutHandle, nullptr); });
}

void App::UpdateComputeDescriptorSets()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.pNext = nullptr;
	descriptorSetAllocInfo.pSetLayouts = &m_ComputeDescriptorSetLayoutHandle;
	descriptorSetAllocInfo.descriptorPool = m_GlobalDescriptorPoolHandle;
	descriptorSetAllocInfo.descriptorSetCount = 1;

	VK_CHECK(vkAllocateDescriptorSets(m_Device->GetLogicalDeviceHandle(), &descriptorSetAllocInfo, &m_ComputeDescriptorSetHandle));

	VkDescriptorBufferInfo descriptorBufferInfos[5];
	descriptorBufferInfos[0].buffer = m_PackedParticlesBufferHandle;
	descriptorBufferInfos[0].offset = m_PosSsboOffset;
	descriptorBufferInfos[0].range = m_PosSsboSize;

	descriptorBufferInfos[1].buffer = m_PackedParticlesBufferHandle;
	descriptorBufferInfos[1].offset = m_VelocitySsboOffset;
	descriptorBufferInfos[1].range = m_VelocitySsboSize;

	descriptorBufferInfos[2].buffer = m_PackedParticlesBufferHandle;
	descriptorBufferInfos[2].offset = m_ForceSsboOffset;
	descriptorBufferInfos[2].range = m_ForceSsboSize;

	descriptorBufferInfos[3].buffer = m_PackedParticlesBufferHandle;
	descriptorBufferInfos[3].offset = m_DensitySsboOffset;
	descriptorBufferInfos[3].range = m_DensitySsboSize;

	descriptorBufferInfos[4].buffer = m_PackedParticlesBufferHandle;
	descriptorBufferInfos[4].offset = m_PressureSsboOffset;
	descriptorBufferInfos[4].range = m_PressureSsboSize;

	VkWriteDescriptorSet writeDescriptorSets[5];
	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].pNext = nullptr;
	writeDescriptorSets[0].dstSet = m_ComputeDescriptorSetHandle;
	writeDescriptorSets[0].dstBinding = 0;
	writeDescriptorSets[0].dstArrayElement = 0;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSets[0].pImageInfo = nullptr;
	writeDescriptorSets[0].pBufferInfo = &descriptorBufferInfos[0];
	writeDescriptorSets[0].pTexelBufferView = nullptr;

	writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[1].pNext = nullptr;
	writeDescriptorSets[1].dstSet = m_ComputeDescriptorSetHandle;
	writeDescriptorSets[1].dstBinding = 0;
	writeDescriptorSets[1].dstArrayElement = 1;
	writeDescriptorSets[1].descriptorCount = 1;
	writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSets[1].pImageInfo = nullptr;
	writeDescriptorSets[1].pBufferInfo = &descriptorBufferInfos[1];
	writeDescriptorSets[1].pTexelBufferView = nullptr;

	writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[2].pNext = nullptr;
	writeDescriptorSets[2].dstSet = m_ComputeDescriptorSetHandle;
	writeDescriptorSets[2].dstBinding = 0;
	writeDescriptorSets[2].dstArrayElement = 2;
	writeDescriptorSets[2].descriptorCount = 1;
	writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSets[2].pImageInfo = nullptr;
	writeDescriptorSets[2].pBufferInfo = &descriptorBufferInfos[2];
	writeDescriptorSets[2].pTexelBufferView = nullptr;

	writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[3].pNext = nullptr;
	writeDescriptorSets[3].dstSet = m_ComputeDescriptorSetHandle;
	writeDescriptorSets[3].dstBinding = 0;
	writeDescriptorSets[3].dstArrayElement = 3;
	writeDescriptorSets[3].descriptorCount = 1;
	writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSets[3].pImageInfo = nullptr;
	writeDescriptorSets[3].pBufferInfo = &descriptorBufferInfos[3];
	writeDescriptorSets[3].pTexelBufferView = nullptr;

	writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[4].pNext = nullptr;
	writeDescriptorSets[4].dstSet = m_ComputeDescriptorSetHandle;
	writeDescriptorSets[4].dstBinding = 0;
	writeDescriptorSets[4].dstArrayElement = 4;
	writeDescriptorSets[4].descriptorCount = 1;
	writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSets[4].pImageInfo = nullptr;
	writeDescriptorSets[4].pBufferInfo = &descriptorBufferInfos[4];
	writeDescriptorSets[4].pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(m_Device->GetLogicalDeviceHandle(), 5, writeDescriptorSets, 0, nullptr);
}

void App::CreateComputePipelineLayout()
{
	VkPipelineLayoutCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.setLayoutCount = 1;
	info.pSetLayouts = &m_ComputeDescriptorSetLayoutHandle;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;

	VK_CHECK(vkCreatePipelineLayout(m_Device->GetLogicalDeviceHandle(), &info, nullptr, &m_ComputePipelineLayoutHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyPipelineLayout(m_Device->GetLogicalDeviceHandle(), m_ComputePipelineLayoutHandle, nullptr); });
}

void App::CreateComputePipelines()
{
	VkShaderModule computeDensityPressureShaderModule = CreateShaderModuleFromSpirvFile(m_Device->GetLogicalDeviceHandle(), "density_pressure.comp.spv");

	VkPipelineShaderStageCreateInfo computeShaderStageCreateInfo{};
	computeShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageCreateInfo.pNext = nullptr;
	computeShaderStageCreateInfo.flags = 0;
	computeShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageCreateInfo.module = computeDensityPressureShaderModule;
	computeShaderStageCreateInfo.pName = "main";
	computeShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.flags = 0;
	pipelineInfo.stage = computeShaderStageCreateInfo;
	pipelineInfo.layout = m_ComputePipelineLayoutHandle;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VK_CHECK(vkCreateComputePipelines(m_Device->GetLogicalDeviceHandle(), m_GlobalPipelineCacheHandle, 1, &pipelineInfo, nullptr, &m_ComputePipelineHandles[0]));

	VkShaderModule computeForceShaderModule = CreateShaderModuleFromSpirvFile(m_Device->GetLogicalDeviceHandle(), "force.comp.spv");

	computeShaderStageCreateInfo.module = computeForceShaderModule;
	pipelineInfo.stage = computeShaderStageCreateInfo;

	VK_CHECK(vkCreateComputePipelines(m_Device->GetLogicalDeviceHandle(), m_GlobalPipelineCacheHandle, 1, &pipelineInfo, nullptr, &m_ComputePipelineHandles[1]));

	VkShaderModule computeIntegrateShaderModule = CreateShaderModuleFromSpirvFile(m_Device->GetLogicalDeviceHandle(), "integrate.comp.spv");
	computeShaderStageCreateInfo.module = computeIntegrateShaderModule;
	pipelineInfo.stage = computeShaderStageCreateInfo;
	VK_CHECK(vkCreateComputePipelines(m_Device->GetLogicalDeviceHandle(), m_GlobalPipelineCacheHandle, 1, &pipelineInfo, nullptr, &m_ComputePipelineHandles[2]));

	vkDestroyShaderModule(m_Device->GetLogicalDeviceHandle(), computeIntegrateShaderModule, nullptr);
	vkDestroyShaderModule(m_Device->GetLogicalDeviceHandle(), computeForceShaderModule, nullptr);
	vkDestroyShaderModule(m_Device->GetLogicalDeviceHandle(), computeDensityPressureShaderModule, nullptr);

	m_DeletionQueue.Add([=]()
						{
							for (const auto &compPipe : m_ComputePipelineHandles)
								vkDestroyPipeline(m_Device->GetLogicalDeviceHandle(), compPipe, nullptr);
						});
}

void App::CreateComputeCommandPool()
{
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	info.queueFamilyIndex = m_Device->GetQueueIndices().computeFamily.value();

	VK_CHECK(vkCreateCommandPool(m_Device->GetLogicalDeviceHandle(), &info, nullptr, &m_ComputeCommandPoolHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyCommandPool(m_Device->GetLogicalDeviceHandle(), m_ComputeCommandPoolHandle, nullptr); });
}

void App::CreateComputeCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = m_ComputeCommandPoolHandle;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(m_Device->GetLogicalDeviceHandle(), &allocInfo, &m_ComputeCommandBufferHandle));

	m_DeletionQueue.Add([=]()
						{ vkFreeCommandBuffers(m_Device->GetLogicalDeviceHandle(), m_ComputeCommandPoolHandle, 1, &m_ComputeCommandBufferHandle); });
}

void App::InitParticleData(std::array<glm::vec2, PARTICLE_NUM> initParticlePosition)
{
	VkBuffer stagingBufferHandle = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferDeviceMemoryHandle = VK_NULL_HANDLE;

	VkBufferCreateInfo stagingBufferCreateInfo{};
	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferCreateInfo.pNext = nullptr;
	stagingBufferCreateInfo.flags = 0;
	stagingBufferCreateInfo.size = m_PackedBufferSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (!m_Device->GetQueueIndices().IsSameFamily())
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	else
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingBufferCreateInfo.queueFamilyIndexCount = 0;
	stagingBufferCreateInfo.pQueueFamilyIndices = nullptr;

	vkCreateBuffer(m_Device->GetLogicalDeviceHandle(), &stagingBufferCreateInfo, nullptr, &stagingBufferHandle);

	VkMemoryRequirements stagingBufferMemoryRequirements;
	vkGetBufferMemoryRequirements(m_Device->GetLogicalDeviceHandle(), stagingBufferHandle, &stagingBufferMemoryRequirements);

	VkMemoryAllocateInfo stagingBufferMemoryAllocInfo;
	stagingBufferMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	stagingBufferMemoryAllocInfo.pNext = nullptr;
	stagingBufferMemoryAllocInfo.allocationSize = stagingBufferMemoryRequirements.size;
	stagingBufferMemoryAllocInfo.memoryTypeIndex = m_Device->FindMemoryType(stagingBufferMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK(vkAllocateMemory(m_Device->GetLogicalDeviceHandle(), &stagingBufferMemoryAllocInfo, nullptr, &stagingBufferDeviceMemoryHandle));

	vkBindBufferMemory(m_Device->GetLogicalDeviceHandle(), stagingBufferHandle, stagingBufferDeviceMemoryHandle, 0);

	void *mappedMemory = nullptr;
	vkMapMemory(m_Device->GetLogicalDeviceHandle(), stagingBufferDeviceMemoryHandle, 0, stagingBufferMemoryRequirements.size, 0, &mappedMemory);

	std::memset(mappedMemory, 0, m_PackedBufferSize);
	std::memcpy(mappedMemory, initParticlePosition.data(), m_PosSsboSize);
	vkUnmapMemory(m_Device->GetLogicalDeviceHandle(), stagingBufferDeviceMemoryHandle);

	VkCommandBuffer copyCommandBufferHandle;
	VkCommandBufferAllocateInfo copyCommandBufferAllocInfo;
	copyCommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	copyCommandBufferAllocInfo.pNext = nullptr;
	copyCommandBufferAllocInfo.commandPool = m_ComputeCommandPoolHandle;
	copyCommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	copyCommandBufferAllocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(m_Device->GetLogicalDeviceHandle(), &copyCommandBufferAllocInfo, &copyCommandBufferHandle));

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(copyCommandBufferHandle, &commandBufferBeginInfo));

	VkBufferCopy bufferCopyRegion;
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = stagingBufferMemoryRequirements.size;

	vkCmdCopyBuffer(copyCommandBufferHandle, stagingBufferHandle, m_PackedParticlesBufferHandle, 1, &bufferCopyRegion);

	VK_CHECK(vkEndCommandBuffer(copyCommandBufferHandle));

	VkSubmitInfo copySubmitInfo;
	copySubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	copySubmitInfo.pNext = nullptr;
	copySubmitInfo.waitSemaphoreCount = 0;
	copySubmitInfo.pWaitSemaphores = nullptr;
	copySubmitInfo.pWaitDstStageMask = 0;
	copySubmitInfo.commandBufferCount = 1;
	copySubmitInfo.pCommandBuffers = &copyCommandBufferHandle;
	copySubmitInfo.signalSemaphoreCount = 0;
	copySubmitInfo.pSignalSemaphores = nullptr;

	VK_CHECK(m_Device->GetComputeQueue()->Submit(1, &copySubmitInfo, VK_NULL_HANDLE));
	VK_CHECK(m_Device->GetComputeQueue()->WaitIdle());

	vkFreeCommandBuffers(m_Device->GetLogicalDeviceHandle(), m_ComputeCommandPoolHandle, 1, &copyCommandBufferHandle);
	vkFreeMemory(m_Device->GetLogicalDeviceHandle(), stagingBufferDeviceMemoryHandle, nullptr);
	vkDestroyBuffer(m_Device->GetLogicalDeviceHandle(), stagingBufferHandle, nullptr);
}