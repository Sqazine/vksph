#include "VKUtils.h"
#include <algorithm>
#include <iostream>


std::vector<VkLayerProperties> GetInstanceLayerProps()
{
	
	uint32_t availableLayerCount;
	vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
	std::vector<VkLayerProperties> availableInstanceLayerProps(availableLayerCount);
	vkEnumerateInstanceLayerProperties(&availableLayerCount, availableInstanceLayerProps.data());

	std::cout << "[INFO]Available Instance Layers:" << std::endl;
	for (const auto& layer : availableInstanceLayerProps)
		std::cout << "[INFO]     name: " << layer.layerName
		<< " impl_ver: "
		<< VK_VERSION_MAJOR(layer.implementationVersion) << "."
		<< VK_VERSION_MINOR(layer.implementationVersion) << "."
		<< VK_VERSION_PATCH(layer.implementationVersion)
		<< " spec_ver: "
		<< VK_VERSION_MAJOR(layer.specVersion) << "."
		<< VK_VERSION_MINOR(layer.specVersion) << "."
		<< VK_VERSION_PATCH(layer.specVersion)
		<< std::endl;
	return availableInstanceLayerProps;
}
std::vector<VkExtensionProperties> GetInstanceExtensionProps()
{
	uint32_t availableInstanceExtCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtCount, nullptr);
	std::vector<VkExtensionProperties> availableInstanceExtProps(availableInstanceExtCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtCount, availableInstanceExtProps.data());

	std::cout << "[INFO]Available Instance Extensions:" << std::endl;
	for (const auto& ext : availableInstanceExtProps)
		std::cout << "[INFO]     name: " << ext.extensionName << " spec_ver: "
		<< VK_VERSION_MAJOR(ext.specVersion) << "."
		<< VK_VERSION_MINOR(ext.specVersion) << "."
		<< VK_VERSION_PATCH(ext.specVersion) << std::endl;

	return availableInstanceExtProps;
}
std::vector<VkExtensionProperties> GetPhysicalDeviceExtensionProps(VkPhysicalDevice device)
{
	uint32_t physicalDeviceExtCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &physicalDeviceExtCount, nullptr);
	std::vector<VkExtensionProperties> phyDeviceExtProps(physicalDeviceExtCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &physicalDeviceExtCount, phyDeviceExtProps.data());

	std::cout << "[INFO]Selected device available extensions:" << std::endl;
	for (const auto& extension : phyDeviceExtProps)
	{
		std::cout << "[INFO]     name: " << extension.extensionName << " spec_ver: "
			<< VK_VERSION_MAJOR(extension.specVersion) << "."
			<< VK_VERSION_MINOR(extension.specVersion) << "."
			<< VK_VERSION_PATCH(extension.specVersion) << std::endl;
	}

	return phyDeviceExtProps;
}
VkPhysicalDeviceProperties GetPhysicalDeviceProps(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);

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
	default:
		;
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

	return props;
}
VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProps(VkPhysicalDevice device)
{
	VkPhysicalDeviceMemoryProperties props;
	vkGetPhysicalDeviceMemoryProperties(device, &props);
	return props;
}
VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures(VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);
	return features;
}
bool CheckValidationLayerSupport(std::vector<const char*> validationLayerNames, std::vector<VkLayerProperties> instanceLayerProps)
{
	for (const char* layerName : validationLayerNames)
	{
		bool layerFound = false;
		for (const auto& layerProperties : instanceLayerProps)
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}

		if (layerFound == false)
			return false;
	}
	return true;
}


QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			indices.computeFamily = i;
		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
		if (present_support)
			indices.presentFamily = i;

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device,VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	details.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	details.presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());

	return details;
}

VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}

VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& avaiablePresentMode : availablePresentModes)
		if (avaiablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return avaiablePresentMode;
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapChainExtent(SDL_Window* window, const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;
	else
	{
		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		VkExtent2D actualExtent = {
			(uint32_t)width,
			(uint32_t)height
		};

		actualExtent.width = std::clamp(actualExtent.width,capabilities.minImageExtent.width,capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height,capabilities.minImageExtent.height,capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

std::vector<VkImage> GetSwapChainImages(VkDevice device, VkSwapchainKHR swapChain)
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	std::vector<VkImage> swapChainImages(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	return swapChainImages;
}

std::vector<VkImageView> CreateSwapChainImageViews(VkDevice device, const std::vector<VkImage>& images, VkFormat format)
{
	std::vector<VkImageView> imageViews(images.size());

	for (size_t i = 0; i < images.size(); ++i)
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.image = images[i];
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = format;
		info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(device,&info,nullptr,&imageViews[i]));
	}
	return imageViews;
}
