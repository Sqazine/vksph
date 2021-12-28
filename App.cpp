#include "App.h"
#include <iostream>
#include "Shader.h"
#include "GraphicsContext.h"
App::App(const WindowCreateInfo &windowInfo)
	: m_IsRunning(true), m_WinInfo(windowInfo)
{
}

App::~App()
{
	VK::GraphicsContext::GetDevice()->WaitIdle();
	VK::GraphicsContext::Destroy();
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

	VK::GraphicsContext::Init(m_WinInfo);

	m_SwapChain = std::make_unique<VK::SwapChain>();
	m_RenderPass = std::make_unique<VK::RenderPass>(m_SwapChain.get());

	for (size_t i = 0; i < m_SwapChain->GetVKSwapChainImageViews().size(); ++i)
	{
		std::vector<VkImageView> views = {m_SwapChain->GetVKSwapChainImageViews()[i]};
		m_SwapChainFrameBuffers.emplace_back(std::make_unique<VK::Framebuffer>(
			m_RenderPass->GetVKRenderPassHandle(),
			views,
			m_SwapChain->GetVKSwapChainExtent().width,
			m_SwapChain->GetVKSwapChainExtent().height));
	}

	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSize.descriptorCount = 5;

	m_GlobalDescriptorPool = std::make_unique<VK::DescriptorPool>(poolSize);

	m_GlobalPipelineCache = std::make_unique<VK::PipelineCache>();

	m_GraphicsPipelineLayout = std::make_unique<VK::PipelineLayout>();

	std::unique_ptr<VK::Shader> vertShader = std::make_unique<VK::Shader>(VK_SHADER_STAGE_VERTEX_BIT, "particle.vert.spv");
	std::unique_ptr<VK::Shader> fragShader = std::make_unique<VK::Shader>(VK_SHADER_STAGE_FRAGMENT_BIT, "particle.frag.spv");

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
	shaderStageCreateInfos.emplace_back(vertShader->GetStageCreateInfo());
	shaderStageCreateInfos.emplace_back(fragShader->GetStageCreateInfo());

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

	m_GraphicsPipeline = std::make_unique<VK::GraphicsPipeline>(
		shaderStageCreateInfos,
		&vertexInputStateInfo,
		&inputAssemblyStateInfo,
		nullptr,
		&viewportStateInfo,
		&rasterizationStateInfo,
		&multiSampleStateInfo,
		nullptr,
		&colorBlendStateInfo,
		nullptr,
		m_GraphicsPipelineLayout.get(),
		m_RenderPass.get(),
		m_GlobalPipelineCache.get());

	m_GraphicsCommandPool = std::make_unique<VK::CommandPool>(VK::GraphicsContext::GetDevice()->GetQueueIndices().graphicsFamily.value());
	m_GraphicsCommandBuffers = std::make_unique<VK::CommandBuffers>(m_GraphicsCommandPool.get(), m_SwapChainFrameBuffers.size());

	m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	m_ImagesInFlight.resize(m_SwapChain->GetVKSwapChainImages().size(), nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		m_ImageAvailableSemaphores[i] = std::make_unique<VK::Semaphore>();
		m_RenderFinishedSemaphores[i] = std::make_unique<VK::Semaphore>();
		m_InFlightFences[i] = std::make_unique<VK::Fence>(VK_FENCE_CREATE_SIGNALED_BIT);
	}

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;

	descriptorSetLayoutBinding.binding = 0;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);

	descriptorSetLayoutBinding.binding = 1;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);

	descriptorSetLayoutBinding.binding = 2;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);

	descriptorSetLayoutBinding.binding = 3;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);

	descriptorSetLayoutBinding.binding = 4;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);

	m_ComputeDescriptorSetLayout = std::make_unique<VK::DescriptorSetLayout>(descriptorSetLayoutBindings);

	std::vector<VkDescriptorSetLayout> compDescSetLayouts = {m_ComputeDescriptorSetLayout->GetVKDescriptorSetLayoutHandle()};
	m_ComputePipelineLayout = std::make_unique<VK::PipelineLayout>(compDescSetLayouts);

	std::unique_ptr<VK::Shader> computeDensityPressureShader = std::make_unique<VK::Shader>(VK_SHADER_STAGE_COMPUTE_BIT, "density_pressure.comp.spv");
	m_ComputePipelines[0] = std::make_unique<VK::ComputePipeline>(computeDensityPressureShader.get(), m_ComputePipelineLayout.get(), m_GlobalPipelineCache.get());

	std::unique_ptr<VK::Shader> computeForceShader = std::make_unique<VK::Shader>(VK_SHADER_STAGE_COMPUTE_BIT, "force.comp.spv");
	m_ComputePipelines[1] = std::make_unique<VK::ComputePipeline>(computeForceShader.get(), m_ComputePipelineLayout.get(), m_GlobalPipelineCache.get());

	std::unique_ptr<VK::Shader> computeIntegrateShader = std::make_unique<VK::Shader>(VK_SHADER_STAGE_COMPUTE_BIT, "integrate.comp.spv");
	m_ComputePipelines[2] = std::make_unique<VK::ComputePipeline>(computeIntegrateShader.get(), m_ComputePipelineLayout.get(), m_GlobalPipelineCache.get());

	m_ComputeCommandPool = std::make_unique<VK::CommandPool>(VK::GraphicsContext::GetDevice()->GetQueueIndices().computeFamily.value());
	m_ComputeCommandBuffers = std::make_unique<VK::CommandBuffers>(m_ComputeCommandPool.get(), 1);

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

	m_PackedParticlesBuffer = std::make_unique<VK::Buffer>(
		m_PackedBufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
	computeSubmitInfo.pCommandBuffers = m_ComputeCommandBuffers->GetVKCommandBuffers().data();
	computeSubmitInfo.signalSemaphoreCount = 0;
	computeSubmitInfo.pSignalSemaphores = nullptr;

	//update particle
	VK::GraphicsContext::GetDevice()->GetComputeQueue()->Submit(1, &computeSubmitInfo, VK_NULL_HANDLE);
}
void App::Simulate()
{
	VkCommandBuffer commandBuffer = m_ComputeCommandBuffers->Begin(0);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineLayout->GetVKPipelineLayoutHandle(), 0, 1, &m_ComputeDescriptorSetHandle, 0, nullptr);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelines[0]->GetVKPipelineHandle());
	vkCmdDispatch(commandBuffer, WORK_GROUP_NUM, 1, 1);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelines[1]->GetVKPipelineHandle());
	vkCmdDispatch(commandBuffer, WORK_GROUP_NUM, 1, 1);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelines[2]->GetVKPipelineHandle());
	vkCmdDispatch(commandBuffer, WORK_GROUP_NUM, 1, 1);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	m_ComputeCommandBuffers->End(commandBuffer);
}

void App::Draw()
{
	for (size_t i = 0; i < m_GraphicsCommandBuffers->GetVKCommandBuffers().size(); ++i)
	{
		VkCommandBuffer commandBuffer = m_GraphicsCommandBuffers->Begin(i);

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

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

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

		vkCmdSetViewport(commandBuffer, 0, 1, &viewPort);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->GetVKPipelineHandle());

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_PackedParticlesBuffer->GetVKBufferHandle(), &offsets);
		vkCmdDraw(commandBuffer, PARTICLE_NUM, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		m_GraphicsCommandBuffers->End(commandBuffer);
	}
}

void App::SubmitAndPresent()
{
	uint32_t swapChainImageIdx = 0;

	vkAcquireNextImageKHR(VK::GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_SwapChain->GetVKSwapChainHandle(), UINT64_MAX, m_ImageAvailableSemaphores[currentFrame]->GetVKSemaphoreHandle(), VK_NULL_HANDLE, &swapChainImageIdx);

	if (m_ImagesInFlight[swapChainImageIdx] != nullptr)
		m_ImagesInFlight[swapChainImageIdx]->Wait(VK_TRUE, UINT64_MAX);
	m_ImagesInFlight[swapChainImageIdx] = m_InFlightFences[currentFrame].get();

	m_InFlightFences[currentFrame]->Reset();

	const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo graphicsSubmitInfo{};
	graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	graphicsSubmitInfo.pNext = nullptr;
	graphicsSubmitInfo.waitSemaphoreCount = 1;
	graphicsSubmitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[currentFrame]->GetVKSemaphoreHandle();
	graphicsSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
	graphicsSubmitInfo.commandBufferCount = 1;
	graphicsSubmitInfo.pCommandBuffers = &m_GraphicsCommandBuffers->GetVKCommandBuffers()[swapChainImageIdx];
	graphicsSubmitInfo.signalSemaphoreCount = 1;
	graphicsSubmitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[currentFrame]->GetVKSemaphoreHandle();

	VK::GraphicsContext::GetDevice()->GetGraphicsQueue()->Submit(1, &graphicsSubmitInfo, m_InFlightFences[currentFrame].get());

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[currentFrame]->GetVKSemaphoreHandle();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_SwapChain->GetVKSwapChainHandle();
	presentInfo.pImageIndices = &swapChainImageIdx;
	presentInfo.pResults = nullptr;

	VK::GraphicsContext::GetDevice()->GetPresentQueue()->Present(&presentInfo);
	VK::GraphicsContext::GetDevice()->GetPresentQueue()->WaitIdle();

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void App::UpdateComputeDescriptorSets()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.pNext = nullptr;
	descriptorSetAllocInfo.pSetLayouts = &m_ComputeDescriptorSetLayout->GetVKDescriptorSetLayoutHandle();
	descriptorSetAllocInfo.descriptorPool = m_GlobalDescriptorPool->GetVKDescriptorPoolHandle();
	descriptorSetAllocInfo.descriptorSetCount = 1;

	VK_CHECK(vkAllocateDescriptorSets(VK::GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &descriptorSetAllocInfo, &m_ComputeDescriptorSetHandle));

	VkDescriptorBufferInfo descriptorBufferInfos[5];
	descriptorBufferInfos[0].buffer = m_PackedParticlesBuffer->GetVKBufferHandle();
	descriptorBufferInfos[0].offset = m_PosSsboOffset;
	descriptorBufferInfos[0].range = m_PosSsboSize;

	descriptorBufferInfos[1].buffer = m_PackedParticlesBuffer->GetVKBufferHandle();
	descriptorBufferInfos[1].offset = m_VelocitySsboOffset;
	descriptorBufferInfos[1].range = m_VelocitySsboSize;

	descriptorBufferInfos[2].buffer = m_PackedParticlesBuffer->GetVKBufferHandle();
	descriptorBufferInfos[2].offset = m_ForceSsboOffset;
	descriptorBufferInfos[2].range = m_ForceSsboSize;

	descriptorBufferInfos[3].buffer = m_PackedParticlesBuffer->GetVKBufferHandle();
	descriptorBufferInfos[3].offset = m_DensitySsboOffset;
	descriptorBufferInfos[3].range = m_DensitySsboSize;

	descriptorBufferInfos[4].buffer = m_PackedParticlesBuffer->GetVKBufferHandle();
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

	vkUpdateDescriptorSets(VK::GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), 5, writeDescriptorSets, 0, nullptr);
}

void App::InitParticleData(std::array<glm::vec2, PARTICLE_NUM> initParticlePosition)
{
	VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (!VK::GraphicsContext::GetDevice()->GetQueueIndices().IsSameFamily())
		sharingMode = VK_SHARING_MODE_CONCURRENT;

	VK::Buffer stagingBuffer = VK::Buffer(m_PackedBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sharingMode, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.Fill(m_PosSsboSize, initParticlePosition.data());

	VK::CommandBuffers copyCommandBuffers = VK::CommandBuffers(m_ComputeCommandPool.get(), 1);

	VkCommandBuffer commandBuffer = copyCommandBuffers.Begin(0);

	VkBufferCopy bufferCopyRegion;
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = stagingBuffer.GetVKBufferSize();

	m_PackedParticlesBuffer->CopyFrom(commandBuffer, bufferCopyRegion, stagingBuffer);

	copyCommandBuffers.End(commandBuffer);

	VkSubmitInfo copySubmitInfo;
	copySubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	copySubmitInfo.pNext = nullptr;
	copySubmitInfo.waitSemaphoreCount = 0;
	copySubmitInfo.pWaitSemaphores = nullptr;
	copySubmitInfo.pWaitDstStageMask = 0;
	copySubmitInfo.commandBufferCount = 1;
	copySubmitInfo.pCommandBuffers = &commandBuffer;
	copySubmitInfo.signalSemaphoreCount = 0;
	copySubmitInfo.pSignalSemaphores = nullptr;

	VK::GraphicsContext::GetDevice()->GetComputeQueue()->Submit(1, &copySubmitInfo);
	VK::GraphicsContext::GetDevice()->GetComputeQueue()->WaitIdle();
}