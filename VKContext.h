#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include "VKUtils.h"

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};



class VKContext
{
public:
	void Init(const struct WindowCreateInfo& info);
	void Destroy();

private:
	void CreateWindow(const struct WindowCreateInfo& info);
	void LoadVulkanLib();
	void CreateInstance();
	void CreateDebugUtilsMessenger();
	void CreateSurface();
	void SelectPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();
	
	void DestroySwapChain();
	void DestroyLogicalDevice();
	void DestroySurface();
	void DestroyDebugUtilMessenger();
	void DestroyInstance();
	void UnLoadVulkanLib();
	void DestroyWindow();


	SDL_Window* m_WindowHandle;

	std::vector<VkExtensionProperties> m_AvailableInstanceExtensionProps;
	std::vector<VkLayerProperties> m_AvailableInstanceLayerProps;
	VkInstance m_InstanceHandle;

	VkDebugUtilsMessengerEXT m_DebugMessengerHandle;

	VkSurfaceKHR m_SurfaceHandle;

	VkPhysicalDevice m_PhysicalDeviceHandle;
	std::vector<VkExtensionProperties> m_PhysicalDeviceExtensionProps;
	VkPhysicalDeviceProperties m_PhysicalDeivceProps;
	VkPhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProps;
	VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;

	QueueFamilyIndices m_QueueIndices;
	VkDevice m_LogicalDeviceHandle;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;
	VkQueue m_ComputeQueue;

	SwapChainSupportDetails m_SwapChainSupportDetails;
	VkSwapchainKHR m_SwapChainHandle;
	std::vector<VkImage> m_SwapChainImages;
	std::vector<VkImageView> m_SwapChainImageViews;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;

};