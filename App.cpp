#include "App.h"
#include <iostream>
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
													VkDebugUtilsMessageTypeFlagsEXT messageType,
													const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
													void *)
{
	std::string ms;

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		ms = "VERBOSE";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		ms = "ERROR";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		ms = "WARNING";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		ms = "INFO";
		break;
	default:
		ms = "UNKNOWN";
		break;
	}

	std::string mt;

	if (messageType == 7)
		mt = "General | Validation | Performance";
	else if (messageType == 6)
		mt = "Validation | Performance";
	else if (messageType == 5)
		mt = "General | Performance";
	else if (messageType == 4)
		mt = "Performance";
	else if (messageType == 3)
		mt = "General | Validation";
	else if (messageType == 2)
		mt = "Validation";
	else if (messageType == 1)
		mt = "General";
	else
		mt = "Unknown";

	printf("[%s: %s]\n%s\n", ms.c_str(), mt.c_str(), pCallbackData->pMessage);

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

App::App(std::string title, int32_t width, int32_t height)
	: m_IsRunning(true), m_WindowWidth(width), m_WindowHeight(height), m_WindowTitle(title)
{
}

App::~App()
{
	vkDeviceWaitIdle(m_LogicalDeviceHandle);

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
	CreateInstance();
#if _DEBUG
	CreateDebugUtilsMessenger();
#endif
	CreateSurface();
	SelectPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateRenderPass();
	CreateSwapChainFrameBuffers();
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
	VK_CHECK(vkQueueSubmit(m_ComputeQueueHandle, 1, &computeSubmitInfo, VK_NULL_HANDLE));
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
		renderPassBeginInfo.renderPass = m_RenderPassHandle;
		renderPassBeginInfo.framebuffer = m_SwapChainFrameBufferHandles[i];
		renderPassBeginInfo.renderArea.offset = {0, 0};
		renderPassBeginInfo.renderArea.extent = m_SwapChainExtent;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(m_GraphicsCommandBufferHandles[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewPort;
		viewPort.x = 0;
		viewPort.y = 0;
		viewPort.width = m_SwapChainExtent.width;
		viewPort.height = m_SwapChainExtent.height;
		viewPort.minDepth = 0;
		viewPort.maxDepth = 1;

		VkRect2D scissor;
		scissor.extent = m_SwapChainExtent;
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
	vkAcquireNextImageKHR(m_LogicalDeviceHandle, m_SwapChainHandle, UINT64_MAX, m_ImageAvailableSemaphoreHandle, VK_NULL_HANDLE, &m_SwapChainImageIndex);

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

	VK_CHECK(vkQueueSubmit(m_GraphicsQueueHandle, 1, &graphicsSubmitInfo, VK_NULL_HANDLE));

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphoreHandle;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_SwapChainHandle;
	presentInfo.pImageIndices = &m_SwapChainImageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(m_PresentQueueHandle, &presentInfo);
	vkQueueWaitIdle(m_PresentQueueHandle);
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

void App::CreateInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;
	appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.pApplicationName = nullptr;
	appInfo.pEngineName = nullptr;

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;

	uint32_t instanceExtCount;
	std::vector<const char *> requiredInstanceExts;
	SDL_Vulkan_GetInstanceExtensions(m_WindowHandle, &instanceExtCount, nullptr);
	requiredInstanceExts.resize(instanceExtCount);
	SDL_Vulkan_GetInstanceExtensions(m_WindowHandle, &instanceExtCount, requiredInstanceExts.data());
#if _DEBUG
	requiredInstanceExts.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	std::vector<VkExtensionProperties> instanceExtensionProps = GetInstanceExtensionProps();

	bool extSatisfied = CheckExtensionSupport(requiredInstanceExts, instanceExtensionProps);

	if (extSatisfied)
	{
		instanceInfo.enabledExtensionCount = requiredInstanceExts.size();
		instanceInfo.ppEnabledExtensionNames = requiredInstanceExts.data();
	}
	else
	{
		instanceInfo.enabledExtensionCount = 0;
		instanceInfo.ppEnabledExtensionNames = nullptr;
	}

#if _DEBUG
	instanceInfo.enabledLayerCount = validationLayers.size();
	instanceInfo.ppEnabledLayerNames = validationLayers.data();
	if (!CheckValidationLayerSupport(validationLayers, GetInstanceLayerProps()))
		std::cout << "Lack of necessary validation layer" << std::endl;
#endif

	VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &m_InstanceHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyInstance(m_InstanceHandle, nullptr); });
}

void App::CreateDebugUtilsMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pUserData = nullptr;

	if (CreateDebugUtilsMessengerEXT(m_InstanceHandle, &createInfo, nullptr, &m_DebugMessengerHandle) != VK_SUCCESS)
		std::cout << "Failed to create debug messenger" << std::endl;

	m_DeletionQueue.Add([=]()
						{ DestroyDebugUtilsMessengerEXT(m_InstanceHandle, m_DebugMessengerHandle, nullptr); });
}

void App::CreateSurface()
{
	if (SDL_Vulkan_CreateSurface(m_WindowHandle, m_InstanceHandle, &m_SurfaceHandle) != SDL_TRUE)
		std::cout << "Failed to create vulkan surface:" << SDL_GetError() << std::endl;

	m_DeletionQueue.Add([=]()
						{ vkDestroySurfaceKHR(m_InstanceHandle, m_SurfaceHandle, nullptr); });
}

void App::SelectPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_InstanceHandle, &physicalDeviceCount, nullptr);
	if (physicalDeviceCount == 0)
		std::cout << "No GPU support vulkan" << std::endl;
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(m_InstanceHandle, &physicalDeviceCount, physicalDevices.data());

	for (auto phyDev : physicalDevices)
	{
		std::vector<VkExtensionProperties> phyDevExtensions = GetPhysicalDeviceExtensionProps(phyDev);
		VkPhysicalDeviceFeatures features = GetPhysicalDeviceFeatures(phyDev);

		bool extSatisfied = CheckExtensionSupport(deviceExtensions, phyDevExtensions);

		if (extSatisfied && FindQueueFamilies(phyDev, m_SurfaceHandle).isComplete())
			m_PhysicalDeviceHandle = phyDev;
		VkPhysicalDeviceProperties props = GetPhysicalDeviceProps(m_PhysicalDeviceHandle);
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			break;
	}

	VkPhysicalDeviceProperties props = GetPhysicalDeviceProps(m_PhysicalDeviceHandle);
	std::vector<VkExtensionProperties> phyDevExtensions = GetPhysicalDeviceExtensionProps(m_PhysicalDeviceHandle);

	std::cout << "[INFO]Selected device name: " << props.deviceName << std::endl
			  << "[INFO]Selected device type: ";
	switch (props.deviceType)
	{
	case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		std::cout << "VK_PHYSICAL_DEVICE_TYPE_OTHER";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		std::cout << "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		std::cout << "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		std::cout << "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		std::cout << "VK_PHYSICAL_DEVICE_TYPE_CPU";
		break;
	default:;
	}
	std::cout << " (" << props.deviceType << ")" << std::endl
			  << "[INFO]Selected device driver version: "
			  << VK_VERSION_MAJOR(props.driverVersion) << "."
			  << VK_VERSION_MINOR(props.driverVersion) << "."
			  << VK_VERSION_PATCH(props.driverVersion) << std::endl
			  << "[INFO]Selected device vulkan api version: "
			  << VK_VERSION_MAJOR(props.apiVersion) << "."
			  << VK_VERSION_MINOR(props.apiVersion) << "."
			  << VK_VERSION_PATCH(props.apiVersion) << std::endl;

	std::cout << "[INFO]Selected device available extensions:" << std::endl;
	for (const auto &extension : phyDevExtensions)
	{
		std::cout << "[INFO]     name: " << extension.extensionName << " spec_ver: "
				  << VK_VERSION_MAJOR(extension.specVersion) << "."
				  << VK_VERSION_MINOR(extension.specVersion) << "."
				  << VK_VERSION_PATCH(extension.specVersion) << std::endl;
	}
}

void App::CreateLogicalDevice()
{
	m_QueueIndices = FindQueueFamilies(m_PhysicalDeviceHandle, m_SurfaceHandle);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	if (m_QueueIndices.IsSameFamily())
	{
		const float queuePriorities[3]{1, 1, 1};
		VkDeviceQueueCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.pQueuePriorities = queuePriorities;
		info.queueCount = 3;
		info.queueFamilyIndex = m_QueueIndices.graphicsFamily.value();
		queueCreateInfos.emplace_back(info);
	}
	else
	{
		const float queuePriorities = 1.0f;
		std::vector<uint32_t> uniIndices = {m_QueueIndices.graphicsFamily.value(), m_QueueIndices.presentFamily.value(), m_QueueIndices.computeFamily.value()};
		for (auto idx : uniIndices)
		{
			VkDeviceQueueCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.pQueuePriorities = &queuePriorities;
			info.queueCount = 1;
			info.queueFamilyIndex = idx;
			queueCreateInfos.emplace_back(info);
		}
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
#if _DEBUG
	deviceCreateInfo.enabledLayerCount = validationLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif
	deviceCreateInfo.pEnabledFeatures = nullptr;
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	VK_CHECK(vkCreateDevice(m_PhysicalDeviceHandle, &deviceCreateInfo, nullptr, &m_LogicalDeviceHandle));

	vkGetDeviceQueue(m_LogicalDeviceHandle, m_QueueIndices.graphicsFamily.value(), 0, &m_GraphicsQueueHandle);
	vkGetDeviceQueue(m_LogicalDeviceHandle, m_QueueIndices.presentFamily.value(), 0, &m_PresentQueueHandle);
	vkGetDeviceQueue(m_LogicalDeviceHandle, m_QueueIndices.computeFamily.value(), 0, &m_ComputeQueueHandle);

	m_DeletionQueue.Add([=]()
						{ vkDestroyDevice(m_LogicalDeviceHandle, nullptr); });
}

void App::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDeviceHandle, m_SurfaceHandle);
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapChainExtent(m_WindowHandle, swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = m_SurfaceHandle;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::vector<uint32_t> uniIndices = {m_QueueIndices.graphicsFamily.value(), m_QueueIndices.presentFamily.value(), m_QueueIndices.computeFamily.value()};

	if (m_QueueIndices.graphicsFamily != m_QueueIndices.presentFamily ||
		m_QueueIndices.graphicsFamily != m_QueueIndices.computeFamily ||
		m_QueueIndices.presentFamily != m_QueueIndices.computeFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = uniIndices.size();
		createInfo.pQueueFamilyIndices = uniIndices.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(m_LogicalDeviceHandle, &createInfo, nullptr, &m_SwapChainHandle));

	m_SwapChainImages = GetSwapChainImages(m_LogicalDeviceHandle, m_SwapChainHandle);
	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtent = extent;
	m_SwapChainImageViews = CreateSwapChainImageViews(m_LogicalDeviceHandle, m_SwapChainImages, m_SwapChainImageFormat);

	m_DeletionQueue.Add([=]()
						{
							for (auto imageView : m_SwapChainImageViews)
								vkDestroyImageView(m_LogicalDeviceHandle, imageView, nullptr);
							vkDestroySwapchainKHR(m_LogicalDeviceHandle, m_SwapChainHandle, nullptr);
						});
}

void App::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VK_CHECK(vkCreateRenderPass(m_LogicalDeviceHandle, &renderPassInfo, nullptr, &m_RenderPassHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyRenderPass(m_LogicalDeviceHandle, m_RenderPassHandle, nullptr); });
}

void App::CreateSwapChainFrameBuffers()
{
	m_SwapChainFrameBufferHandles.resize(m_SwapChainImageViews.size());

	for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
	{
		VkFramebufferCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.renderPass = m_RenderPassHandle;
		info.attachmentCount = 1;
		info.pAttachments = &m_SwapChainImageViews[i];
		info.width = m_SwapChainExtent.width;
		info.height = m_SwapChainExtent.height;
		info.layers = 1;

		VK_CHECK(vkCreateFramebuffer(m_LogicalDeviceHandle, &info, nullptr, &m_SwapChainFrameBufferHandles[i]));
	}

	m_DeletionQueue.Add([=]()
						{
							for (auto fb : m_SwapChainFrameBufferHandles)
								vkDestroyFramebuffer(m_LogicalDeviceHandle, fb, nullptr);
						});
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

	VK_CHECK(vkCreateDescriptorPool(m_LogicalDeviceHandle, &poolInfo, nullptr, &m_GlobalDescriptorPoolHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyDescriptorPool(m_LogicalDeviceHandle, m_GlobalDescriptorPoolHandle, nullptr); });
}

void App::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.initialDataSize = 0;
	info.pInitialData = nullptr;

	VK_CHECK(vkCreatePipelineCache(m_LogicalDeviceHandle, &info, nullptr, &m_GlobalPipelineCacheHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyPipelineCache(m_LogicalDeviceHandle, m_GlobalPipelineCacheHandle, nullptr); });
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

	VK_CHECK(vkCreateBuffer(m_LogicalDeviceHandle, &bufferInfo, nullptr, &m_PackedParticlesBufferHandle));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_LogicalDeviceHandle, m_PackedParticlesBufferHandle, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(m_PhysicalDeviceHandle, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vkAllocateMemory(m_LogicalDeviceHandle, &allocInfo, nullptr, &m_PackedParticleBufferMemoryHandle));

	vkBindBufferMemory(m_LogicalDeviceHandle, m_PackedParticlesBufferHandle, m_PackedParticleBufferMemoryHandle, 0);

	m_DeletionQueue.Add([=]()
						{
							vkDestroyBuffer(m_LogicalDeviceHandle, m_PackedParticlesBufferHandle, nullptr);
							vkFreeMemory(m_LogicalDeviceHandle, m_PackedParticleBufferMemoryHandle, nullptr);
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

	VK_CHECK(vkCreatePipelineLayout(m_LogicalDeviceHandle, &info, nullptr, &m_GraphicsPipelineLayoutHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyPipelineLayout(m_LogicalDeviceHandle, m_GraphicsPipelineLayoutHandle, nullptr); });
}

void App::CreateGraphicsPipeline()
{
	VkShaderModule vertShaderModule = CreateShaderModuleFromSpirvFile(m_LogicalDeviceHandle, "particle.vert.spv");
	VkShaderModule fragShaderModule = CreateShaderModuleFromSpirvFile(m_LogicalDeviceHandle, "particle.frag.spv");

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
	viewPort.width = (float)m_SwapChainExtent.width;
	viewPort.height = (float)m_SwapChainExtent.height;
	viewPort.minDepth = 0;
	viewPort.maxDepth = 1;

	VkRect2D scissor = {};
	scissor.extent = m_SwapChainExtent;
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

	VkGraphicsPipelineCreateInfo graphicsPIpelineCreateInfo;
	graphicsPIpelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPIpelineCreateInfo.pNext = nullptr;
	graphicsPIpelineCreateInfo.flags = 0;
	graphicsPIpelineCreateInfo.stageCount = shaderStageCreateInfos.size();
	graphicsPIpelineCreateInfo.pStages = shaderStageCreateInfos.data();
	graphicsPIpelineCreateInfo.pVertexInputState = &vertexInputStateInfo;
	graphicsPIpelineCreateInfo.pInputAssemblyState = &inputAssemblyStateInfo;
	graphicsPIpelineCreateInfo.pTessellationState = nullptr;
	graphicsPIpelineCreateInfo.pViewportState = &viewportStateInfo;
	graphicsPIpelineCreateInfo.pRasterizationState = &rasterizationStateInfo;
	graphicsPIpelineCreateInfo.pMultisampleState = &multiSampleStateInfo;
	graphicsPIpelineCreateInfo.pDepthStencilState = nullptr;
	graphicsPIpelineCreateInfo.pColorBlendState = &colorBlendStateInfo;
	graphicsPIpelineCreateInfo.pDynamicState = nullptr;
	graphicsPIpelineCreateInfo.layout = m_GraphicsPipelineLayoutHandle;
	graphicsPIpelineCreateInfo.renderPass = m_RenderPassHandle;
	graphicsPIpelineCreateInfo.subpass = 0;
	graphicsPIpelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPIpelineCreateInfo.basePipelineIndex = -1;

	VK_CHECK(vkCreateGraphicsPipelines(m_LogicalDeviceHandle, m_GlobalPipelineCacheHandle, 1, &graphicsPIpelineCreateInfo, nullptr, &m_GraphicePipelineHandle));

	vkDestroyShaderModule(m_LogicalDeviceHandle, vertShaderModule, nullptr);
	vkDestroyShaderModule(m_LogicalDeviceHandle, fragShaderModule, nullptr);

	m_DeletionQueue.Add([=]()
						{ vkDestroyPipeline(m_LogicalDeviceHandle, m_GraphicePipelineHandle, nullptr); });
}

void App::CreateGraphicsCommandPool()
{
	VkCommandPoolCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.queueFamilyIndex = m_QueueIndices.graphicsFamily.value();

	VK_CHECK(vkCreateCommandPool(m_LogicalDeviceHandle, &info, nullptr, &m_GraphicsCommandPoolHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyCommandPool(m_LogicalDeviceHandle, m_GraphicsCommandPoolHandle, nullptr); });
}

void App::CreateGraphicsCommandBuffers()
{
	m_GraphicsCommandBufferHandles.resize(m_SwapChainFrameBufferHandles.size());
	VkCommandBufferAllocateInfo bufferAllocInfo = {};
	bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocInfo.pNext = nullptr;
	bufferAllocInfo.commandBufferCount = (uint32_t)m_GraphicsCommandBufferHandles.size();
	bufferAllocInfo.commandPool = m_GraphicsCommandPoolHandle;
	bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VK_CHECK(vkAllocateCommandBuffers(m_LogicalDeviceHandle, &bufferAllocInfo, m_GraphicsCommandBufferHandles.data()));

	m_DeletionQueue.Add([=]()
						{ vkFreeCommandBuffers(m_LogicalDeviceHandle, m_GraphicsCommandPoolHandle, m_GraphicsCommandBufferHandles.size(), m_GraphicsCommandBufferHandles.data()); });
}

void App::CreateSemaphores()
{
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;

	VK_CHECK(vkCreateSemaphore(m_LogicalDeviceHandle, &info, nullptr, &m_ImageAvailableSemaphoreHandle));
	VK_CHECK(vkCreateSemaphore(m_LogicalDeviceHandle, &info, nullptr, &m_RenderFinishedSemaphoreHandle));

	m_DeletionQueue.Add([=]()
						{
							vkDestroySemaphore(m_LogicalDeviceHandle, m_RenderFinishedSemaphoreHandle, nullptr);
							vkDestroySemaphore(m_LogicalDeviceHandle, m_ImageAvailableSemaphoreHandle, nullptr);
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

	VK_CHECK(vkCreateDescriptorSetLayout(m_LogicalDeviceHandle, &descriptorSetLayoutInfo, nullptr, &m_ComputeDescriptorSetLayoutHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyDescriptorSetLayout(m_LogicalDeviceHandle, m_ComputeDescriptorSetLayoutHandle, nullptr); });
}

void App::UpdateComputeDescriptorSets()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.pNext = nullptr;
	descriptorSetAllocInfo.pSetLayouts = &m_ComputeDescriptorSetLayoutHandle;
	descriptorSetAllocInfo.descriptorPool = m_GlobalDescriptorPoolHandle;
	descriptorSetAllocInfo.descriptorSetCount = 1;

	VK_CHECK(vkAllocateDescriptorSets(m_LogicalDeviceHandle, &descriptorSetAllocInfo, &m_ComputeDescriptorSetHandle));

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

	vkUpdateDescriptorSets(m_LogicalDeviceHandle, 5, writeDescriptorSets, 0, nullptr);
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

	VK_CHECK(vkCreatePipelineLayout(m_LogicalDeviceHandle, &info, nullptr, &m_ComputePipelineLayoutHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyPipelineLayout(m_LogicalDeviceHandle, m_ComputePipelineLayoutHandle, nullptr); });
}

void App::CreateComputePipelines()
{
	VkShaderModule computeDensityPressureShaderModule = CreateShaderModuleFromSpirvFile(m_LogicalDeviceHandle, "density_pressure.comp.spv");

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

	VK_CHECK(vkCreateComputePipelines(m_LogicalDeviceHandle, m_GlobalPipelineCacheHandle, 1, &pipelineInfo, nullptr, &m_ComputePipelineHandles[0]));

	VkShaderModule computeForceShaderModule = CreateShaderModuleFromSpirvFile(m_LogicalDeviceHandle, "force.comp.spv");

	computeShaderStageCreateInfo.module = computeForceShaderModule;
	pipelineInfo.stage = computeShaderStageCreateInfo;

	VK_CHECK(vkCreateComputePipelines(m_LogicalDeviceHandle, m_GlobalPipelineCacheHandle, 1, &pipelineInfo, nullptr, &m_ComputePipelineHandles[1]));

	VkShaderModule computeIntegrateShaderModule = CreateShaderModuleFromSpirvFile(m_LogicalDeviceHandle, "integrate.comp.spv");
	computeShaderStageCreateInfo.module = computeIntegrateShaderModule;
	pipelineInfo.stage = computeShaderStageCreateInfo;
	VK_CHECK(vkCreateComputePipelines(m_LogicalDeviceHandle, m_GlobalPipelineCacheHandle, 1, &pipelineInfo, nullptr, &m_ComputePipelineHandles[2]));

	vkDestroyShaderModule(m_LogicalDeviceHandle, computeIntegrateShaderModule, nullptr);
	vkDestroyShaderModule(m_LogicalDeviceHandle, computeForceShaderModule, nullptr);
	vkDestroyShaderModule(m_LogicalDeviceHandle, computeDensityPressureShaderModule, nullptr);

	m_DeletionQueue.Add([=]()
						{
							for (const auto &compPipe : m_ComputePipelineHandles)
								vkDestroyPipeline(m_LogicalDeviceHandle, compPipe, nullptr);
						});
}

void App::CreateComputeCommandPool()
{
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	info.queueFamilyIndex = m_QueueIndices.computeFamily.value();

	VK_CHECK(vkCreateCommandPool(m_LogicalDeviceHandle, &info, nullptr, &m_ComputeCommandPoolHandle));

	m_DeletionQueue.Add([=]()
						{ vkDestroyCommandPool(m_LogicalDeviceHandle, m_ComputeCommandPoolHandle, nullptr); });
}

void App::CreateComputeCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = m_ComputeCommandPoolHandle;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(m_LogicalDeviceHandle, &allocInfo, &m_ComputeCommandBufferHandle));

	m_DeletionQueue.Add([=]()
						{ vkFreeCommandBuffers(m_LogicalDeviceHandle, m_ComputeCommandPoolHandle, 1, &m_ComputeCommandBufferHandle); });
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
	if (m_QueueIndices.graphicsFamily != m_QueueIndices.presentFamily ||
		m_QueueIndices.graphicsFamily != m_QueueIndices.computeFamily ||
		m_QueueIndices.presentFamily != m_QueueIndices.computeFamily)
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	else
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingBufferCreateInfo.queueFamilyIndexCount = 0;
	stagingBufferCreateInfo.pQueueFamilyIndices = nullptr;

	vkCreateBuffer(m_LogicalDeviceHandle, &stagingBufferCreateInfo, nullptr, &stagingBufferHandle);

	VkMemoryRequirements stagingBufferMemoryRequirements;
	vkGetBufferMemoryRequirements(m_LogicalDeviceHandle, stagingBufferHandle, &stagingBufferMemoryRequirements);

	VkMemoryAllocateInfo stagingBufferMemoryAllocInfo;
	stagingBufferMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	stagingBufferMemoryAllocInfo.pNext = nullptr;
	stagingBufferMemoryAllocInfo.allocationSize = stagingBufferMemoryRequirements.size;
	stagingBufferMemoryAllocInfo.memoryTypeIndex = FindMemoryType(m_PhysicalDeviceHandle, stagingBufferMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK(vkAllocateMemory(m_LogicalDeviceHandle, &stagingBufferMemoryAllocInfo, nullptr, &stagingBufferDeviceMemoryHandle));

	vkBindBufferMemory(m_LogicalDeviceHandle, stagingBufferHandle, stagingBufferDeviceMemoryHandle, 0);

	void *mappedMemory = nullptr;
	vkMapMemory(m_LogicalDeviceHandle, stagingBufferDeviceMemoryHandle, 0, stagingBufferMemoryRequirements.size, 0, &mappedMemory);

	std::memset(mappedMemory, 0, m_PackedBufferSize);
	std::memcpy(mappedMemory, initParticlePosition.data(), m_PosSsboSize);
	vkUnmapMemory(m_LogicalDeviceHandle, stagingBufferDeviceMemoryHandle);

	VkCommandBuffer copyCommandBufferHandle;
	VkCommandBufferAllocateInfo copyCommandBufferAllocInfo;
	copyCommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	copyCommandBufferAllocInfo.pNext = nullptr;
	copyCommandBufferAllocInfo.commandPool = m_ComputeCommandPoolHandle;
	copyCommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	copyCommandBufferAllocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(m_LogicalDeviceHandle, &copyCommandBufferAllocInfo, &copyCommandBufferHandle));

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

	VK_CHECK(vkQueueSubmit(m_ComputeQueueHandle, 1, &copySubmitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(m_ComputeQueueHandle));

	vkFreeCommandBuffers(m_LogicalDeviceHandle, m_ComputeCommandPoolHandle, 1, &copyCommandBufferHandle);
	vkFreeMemory(m_LogicalDeviceHandle, stagingBufferDeviceMemoryHandle, nullptr);
	vkDestroyBuffer(m_LogicalDeviceHandle, stagingBufferHandle, nullptr);
}