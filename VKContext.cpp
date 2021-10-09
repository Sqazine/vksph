#include "VKContext.h"
#include "App.h"
#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*)
{
	std::string ms;

	switch (messageSeverity) {
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

	if (messageType == 7) mt = "General | Validation | Performance";
	else if (messageType == 6) mt = "Validation | Performance";
	else if (messageType == 5) mt = "General | Performance";
	else if (messageType == 4) mt = "Performance";
	else if (messageType == 3) mt = "General | Validation";
	else if (messageType == 2) mt = "Validation";
	else if (messageType == 1) mt = "General";
	else mt = "Unknown";

	printf("[%s: %s]\n%s\n", ms.c_str(), mt.c_str(), pCallbackData->pMessage);

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;

}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

void VKContext::Init(const WindowCreateInfo& info)
{
	CreateWindow(info);
	LoadVulkanLib();
	CreateInstance();
#if _DEBUG
	CreateDebugUtilsMessenger();
#endif
	CreateSurface();
	SelectPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
}
void VKContext::Destroy()
{
	DestroySwapChain();
	DestroyLogicalDevice();
	DestroySurface();
	DestroyDebugUtilMessenger();
	DestroyInstance();
	UnLoadVulkanLib();
	DestroyWindow();
}

void VKContext::CreateWindow(const WindowCreateInfo& info)
{
	uint32_t windowFlag = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;
	if (info.resizeable)
		windowFlag |= SDL_WINDOW_RESIZABLE;

	m_WindowHandle = SDL_CreateWindow(info.title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		info.width,
		info.height,
		windowFlag);
}

void VKContext::LoadVulkanLib()
{

	int flag = SDL_Vulkan_LoadLibrary(nullptr);
	if (flag != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Not Support vulkan:%s\n", SDL_GetError());
		exit(1);
	}

	void* instanceProcAddr = SDL_Vulkan_GetVkGetInstanceProcAddr();
	if (!instanceProcAddr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"SDL_Vulkan_GetVkGetInstanceProcAddr(): %s\n",
			SDL_GetError());
		exit(1);
	}
}

void VKContext::CreateInstance()
{
	m_AvailableInstanceExtensionProps = GetInstanceExtensionProps();
	m_AvailableInstanceLayerProps = GetInstanceLayerProps();

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
	std::vector<const char*> requiredInstanceExts;
	SDL_Vulkan_GetInstanceExtensions(m_WindowHandle, &instanceExtCount, nullptr);
	requiredInstanceExts.resize(instanceExtCount);
	SDL_Vulkan_GetInstanceExtensions(m_WindowHandle, &instanceExtCount, requiredInstanceExts.data());

#if _DEBUG
	requiredInstanceExts.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	instanceInfo.enabledLayerCount = validationLayers.size();
	instanceInfo.ppEnabledLayerNames = validationLayers.data();
	if (!CheckValidationLayerSupport(validationLayers, m_AvailableInstanceLayerProps))
		std::cout << "Lack of necessary validation layer" << std::endl;
#endif

	instanceInfo.enabledExtensionCount = requiredInstanceExts.size();
	instanceInfo.ppEnabledExtensionNames = requiredInstanceExts.data();

	VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &m_InstanceHandle));
}

void VKContext::CreateDebugUtilsMessenger()
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
}

void VKContext::CreateSurface()
{
	if (SDL_Vulkan_CreateSurface(m_WindowHandle, m_InstanceHandle, &m_SurfaceHandle) != SDL_TRUE)
		std::cout << "Failed to create vulkan surface:" << SDL_GetError() << std::endl;
}

void VKContext::SelectPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_InstanceHandle, &physicalDeviceCount, nullptr);
	if (physicalDeviceCount == 0)
		std::cout << "No GPU support vulkan" << std::endl;
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(m_InstanceHandle, &physicalDeviceCount, physicalDevices.data());

	m_PhysicalDeviceHandle = physicalDevices.front();//Select the first one

	m_PhysicalDeivceProps = GetPhysicalDeviceProps(m_PhysicalDeviceHandle);
	m_PhysicalDeviceExtensionProps = GetPhysicalDeviceExtensionProps(m_PhysicalDeviceHandle);
	m_PhysicalDeviceFeatures = GetPhysicalDeviceFeatures(m_PhysicalDeviceHandle);
	m_PhysicalDeviceMemoryProps = GetPhysicalDeviceMemoryProps(m_PhysicalDeviceHandle);
}

void VKContext::CreateLogicalDevice()
{
	m_QueueIndices = FindQueueFamilies(m_PhysicalDeviceHandle, m_SurfaceHandle);


	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	if (m_QueueIndices.IsSameFamily())
	{
		const float queuePriorities[3]{ 1,1,1 };
		VkDeviceQueueCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.pQueuePriorities = queuePriorities;
		info.queueCount = 3;
		info.queueFamilyIndex = m_QueueIndices.graphicsFamily.value();
		queueCreateInfos.emplace_back(info);
	}
	else {
		const float queuePriorities = 1.0f;
		std::vector<uint32_t> uniIndices = { m_QueueIndices.graphicsFamily.value(),m_QueueIndices.presentFamily.value(),m_QueueIndices.computeFamily.value() };
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
	deviceCreateInfo.pEnabledFeatures = &m_PhysicalDeviceFeatures;
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	VK_CHECK(vkCreateDevice(m_PhysicalDeviceHandle, &deviceCreateInfo, nullptr, &m_LogicalDeviceHandle));

	vkGetDeviceQueue(m_LogicalDeviceHandle, m_QueueIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_LogicalDeviceHandle, m_QueueIndices.presentFamily.value(), 0, &m_PresentQueue);
	vkGetDeviceQueue(m_LogicalDeviceHandle, m_QueueIndices.computeFamily.value(), 0, &m_ComputeQueue);
}

void VKContext::CreateSwapChain()
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

	std::vector<uint32_t> uniIndices = { m_QueueIndices.graphicsFamily.value(),m_QueueIndices.presentFamily.value(),m_QueueIndices.computeFamily.value() };

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
}

void VKContext::DestroySwapChain()
{
	for (auto imageView : m_SwapChainImageViews)
		vkDestroyImageView(m_LogicalDeviceHandle, imageView, nullptr);
	vkDestroySwapchainKHR(m_LogicalDeviceHandle, m_SwapChainHandle, nullptr);
}

void VKContext::DestroyLogicalDevice()
{
	vkDestroyDevice(m_LogicalDeviceHandle, nullptr);
}

void VKContext::DestroySurface()
{
	vkDestroySurfaceKHR(m_InstanceHandle, m_SurfaceHandle, nullptr);
}

void VKContext::DestroyDebugUtilMessenger()
{
	DestroyDebugUtilsMessengerEXT(m_InstanceHandle, m_DebugMessengerHandle, nullptr);
}

void VKContext::DestroyInstance()
{
	vkDestroyInstance(m_InstanceHandle, nullptr);
}

void VKContext::UnLoadVulkanLib()
{
	SDL_Vulkan_UnloadLibrary();
}

void VKContext::DestroyWindow()
{
	SDL_DestroyWindow(m_WindowHandle);
}
