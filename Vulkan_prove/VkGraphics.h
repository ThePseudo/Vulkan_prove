#pragma once

// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <cstdlib>
#include <vector>
#include <sstream>

#include "Window.h"
#include "common.h"
#include "Camera.h"
#include "ShaderFactory.h"

class VkGraphics
{
public:
	VkGraphics();
	~VkGraphics();

	void onResize();

private:
	// Private structs
	typedef struct {
		vk::Buffer buffer;
		vk::DeviceMemory memory;
		vk::DescriptorBufferInfo info;
	} uniform_data;

	typedef struct _swapchains_buffer {
		vk::Image image;
		vk::ImageView view;
	} swapchains_buffer;

	typedef struct {
		vk::Format format;
		vk::Image image;
		vk::ImageView imageView;
		vk::DeviceMemory deviceMemory;
	} depth;

	// Private functions
	vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
	std::vector<const char*> getAvailableWSIExtensions();
	void vkInit();
	void vkEnumerateDevices();
	void vkCreateDevice();
	void vkCreateCommandBuffer();
	void vkCreateSwapchain();
	void vkInitDepthBuffer();
	void vkInitUniformBuffer();
	void vkInitPipelineLayout();
	void vkInitDescriptorSet();
	void vkCreateRenderPass();
	void vkInitFrameBuffer();

	void vkDestroySwapchain();

	// Utils
	void init();
	void destroy();
	std::vector<vk::ExtensionProperties> getExtensionsProperties();
	bool memoryTypeFromProperties(uint32_t typeBits, vk::MemoryPropertyFlags requirement_mask, uint32_t *typeIndex);
	// Private data
	Window									_window;

	vk::Instance							_instance;
	std::vector<vk::ExtensionProperties>	_extensionProperties;
	std::vector<const char*>				_extensions;
	vk::SurfaceKHR							_surface;
	vk::SwapchainKHR						_swapchain;
	std::vector<swapchains_buffer>			_buffers;

	depth									_depth;

	std::vector<vk::PhysicalDevice>			_gpus;
	vk::Device								_device;
	std::vector<vk::QueueFamilyProperties>	_queue_properties;
	uint32_t								_queue_family_count;
	uint32_t								_graphics_queue_family_index;
	uint32_t								_present_queue_family_index;
	uint32_t								_swapchain_image_count;
	uint32_t								_current_buffer;
	std::vector<vk::DescriptorSetLayout>	_desc_layout;
	vk::PipelineLayout						_pipeline_layout;
	vk::DescriptorPool						_descriptor_pool;
	std::vector<vk::DescriptorSet>			_descriptor_sets;

	vk::CommandBuffer						_command_buffer;
	vk::CommandPool							_command_pool;
	vk::Format								_format;
	vk::PhysicalDeviceMemoryProperties		_memory_properties;
	vk::PhysicalDeviceProperties			_device_properties;
	vk::RenderPass							_render_pass;

	vk::Semaphore							_image_acquired_semaphore;	// No render until image is available

	uniform_data							_uniform_data;
	Camera									_camera; // The default camera

	std::vector<vk::Framebuffer>			_frame_buffers;

	// SHADER TEST
	vk::ShaderModule						_base_vertex_shader;
	vk::ShaderModule						_base_fragment_shader;
};