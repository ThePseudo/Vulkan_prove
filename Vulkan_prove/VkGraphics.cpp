#include "VkGraphics.h"

using namespace glm;


VkGraphics::VkGraphics()
{
	// Create window
	_window.createWindow();
	init();
}


VkGraphics::~VkGraphics()
{
	destroy();
}

vk::SurfaceKHR VkGraphics::createVulkanSurface(const vk::Instance & instance, SDL_Window * window)
{
	switch (_window.info().subsystem) {

#if defined(SDL_VIDEO_DRIVER_ANDROID) && defined(VK_USE_PLATFORM_ANDROID_KHR)
	case SDL_SYSWM_ANDROID: {
		vk::AndroidSurfaceCreateInfoKHR surfaceInfo = vk::AndroidSurfaceCreateInfoKHR()
			.setWindow(windowInfo.info.android.window);
		return instance.createAndroidSurfaceKHR(surfaceInfo);
	}
#endif

#if defined(SDL_VIDEO_DRIVER_MIR) && defined(VK_USE_PLATFORM_MIR_KHR)
	case SDL_SYSWM_MIR: {
		vk::MirSurfaceCreateInfoKHR surfaceInfo = vk::MirSurfaceCreateInfoKHR()
			.setConnection(windowInfo.info.mir.connection)
			.setMirSurface(windowInfo.info.mir.surface);
		return instance.createMirSurfaceKHR(surfaceInfo);
	}
#endif

#if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
	case SDL_SYSWM_WAYLAND: {
		vk::WaylandSurfaceCreateInfoKHR surfaceInfo = vk::WaylandSurfaceCreateInfoKHR()
			.setDisplay(windowInfo.info.wl.display)
			.setSurface(windowInfo.info.wl.surface);
		return instance.createWaylandSurfaceKHR(surfaceInfo);
	}
#endif

#if defined(SDL_VIDEO_DRIVER_WINDOWS) && defined(VK_USE_PLATFORM_WIN32_KHR)
	case SDL_SYSWM_WINDOWS: {
		vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
			.setHinstance(_window.info().info.win.hinstance)
			.setHwnd(_window.info().info.win.window);
		return instance.createWin32SurfaceKHR(surfaceInfo);
	}
#endif

#if defined(SDL_VIDEO_DRIVER_X11) && defined(VK_USE_PLATFORM_XLIB_KHR)
	case SDL_SYSWM_X11: {
		vk::XlibSurfaceCreateInfoKHR surfaceInfo = vk::XlibSurfaceCreateInfoKHR()
			.setDpy(windowInfo.info.x11.display)
			.setWindow(windowInfo.info.x11.window);
		return instance.createXlibSurfaceKHR(surfaceInfo);
	}
#endif

	default:
		throw std::system_error(std::error_code(), "Unsupported window manager is in use.");
	}
}

std::vector<const char*> VkGraphics::getAvailableWSIExtensions()
{
	std::vector<const char*> extensions;
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
	extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
	return extensions;
}

void VkGraphics::vkInit()
{
	// Use validation layers if this is a debug build, and use WSI extensions regardless
	_extensions = getAvailableWSIExtensions();
	std::vector<const char*> layers;
#if defined(_DEBUG)
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
	for (auto i = _extensions.begin(); i != _extensions.end(); ++i) {
		std::cout << *i << std::endl;
	}
#endif
	// vk::ApplicationInfo allows the programmer to specifiy some basic information about the
	// program, which can be useful for layers and tools to provide more debug information.
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setPApplicationName("Vulkan C++ Windowed Program")
		.setApplicationVersion(1)
		.setPEngineName("LunarG SDK")
		.setEngineVersion(1)
		.setApiVersion(VK_API_VERSION_1_1);

	// vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
	// are needed.
	vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32_t>(_extensions.size()))
		.setPpEnabledExtensionNames(_extensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
		.setPpEnabledLayerNames(layers.data());
	// Create the Vulkan instance.
	try {
		_instance = vk::createInstance(instInfo, nullptr);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
		exit(1);
	}

	// Create a Vulkan surface for rendering
	try {
		_surface = createVulkanSurface(_instance, _window.window());
	}
	catch (const std::exception& e) {
		std::cout << "Failed to create Vulkan surface: " << e.what() << std::endl;
		_instance.destroy();
		exit(1);
	}
}

void VkGraphics::vkEnumerateDevices()
{
	uint32_t numDevices;
	try {
		_instance.enumeratePhysicalDevices(&numDevices, nullptr);
		_gpus.resize(numDevices);
		_instance.enumeratePhysicalDevices(&numDevices, _gpus.data());
		for (auto i = _gpus.begin(); i != _gpus.end(); ++i) {
			auto properties1 = i->getProperties();
			for (auto j = ++i; j != _gpus.end(); ++j) {
				auto properties2 = j->getProperties();
				if (properties1.apiVersion < properties2.apiVersion) {
					auto temp = i;
					i = j;
					j = temp;
				}
				else if (properties1.limits.maxMemoryAllocationCount < properties2.limits.maxMemoryAllocationCount) {
					auto temp = i;
					i = j;
					j = temp;
				}
			}
			--i;
		}
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void VkGraphics::vkCreateDevice()
{
	_gpus[0].getQueueFamilyProperties(&_queue_family_count, nullptr);
	_queue_properties.resize(_queue_family_count);
	_gpus[0].getQueueFamilyProperties(&_queue_family_count, _queue_properties.data());
	bool found = false;
	vk::DeviceQueueCreateInfo queue_info = vk::DeviceQueueCreateInfo();
	for (uint32_t i = 0; i < _queue_family_count; ++i) {
		if (_queue_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			found = true;
			queue_info.setQueueFamilyIndex(i);
			_graphics_queue_family_index = i;
			break;
		}
	}
	float queue_priorities[1] = { 0.0 };
	queue_info.setPNext(nullptr)
		.setQueueCount(1)
		.setPQueuePriorities(queue_priorities)
		.setQueueFamilyIndex(_graphics_queue_family_index);
	std::vector<const char*> extensions;
	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);


	vk::DeviceCreateInfo device_info = vk::DeviceCreateInfo();
	device_info.setPNext(nullptr)
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queue_info)
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data())
		.setEnabledLayerCount(0)
		.setPpEnabledLayerNames(nullptr)
		.setPEnabledFeatures(nullptr);

	try {
		_gpus[0].createDevice(&device_info, nullptr, &_device);
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
	}

	_gpus[0].getMemoryProperties(&_memory_properties);
	_gpus[0].getProperties(&_device_properties);
}

void VkGraphics::vkCreateCommandBuffer()
{
	// Creating command pool
	vk::CommandPoolCreateInfo info = vk::CommandPoolCreateInfo();
	info.setPNext(nullptr)
		.setQueueFamilyIndex(_graphics_queue_family_index)
		.setFlags(vk::CommandPoolCreateFlags());
	try {
		_device.createCommandPool(&info, nullptr, &_command_pool);
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	// Creating command buffer
	vk::CommandBufferAllocateInfo bufferInfo = vk::CommandBufferAllocateInfo();
	bufferInfo.setCommandPool(_command_pool)
		.setPNext(nullptr)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);

	try {
		_device.allocateCommandBuffers(&bufferInfo, &_command_buffer);
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void VkGraphics::vkCreateSwapchain()
{
	// Iterate over each queue to learn whether it supports presenting:
	std::vector<vk::Bool32>supportsPresent;
	supportsPresent.resize(_queue_family_count);
	for (uint32_t i = 0; i < _queue_family_count; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(_gpus[0], i, _surface, &supportsPresent[i]);
	}

	_graphics_queue_family_index = UINT32_MAX;
	_present_queue_family_index = UINT32_MAX;
	for (uint32_t i = 0; i < _queue_family_count; ++i)
	{
		if (_queue_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			if (_graphics_queue_family_index == UINT32_MAX) _graphics_queue_family_index = i;
			if (supportsPresent[i])
			{
				_graphics_queue_family_index = i;
				_present_queue_family_index = i;
				break;
			}
		}
	}

	// If didn't find a queue that supports both graphics and present, then
	// find a separate present queue.
	if (_present_queue_family_index == UINT32_MAX)
	{
		for (uint32_t i = 0; i < _queue_family_count; ++i)
		{
			if (supportsPresent[i])
			{
				_present_queue_family_index = i;
			}
		}
	}

	if (_graphics_queue_family_index == UINT32_MAX || _present_queue_family_index == UINT32_MAX)
	{
		std::cout << "Could not find a queues for graphics and "
			"present\n";
		exit(-1);
	}

	// List of vk::Formats supported
	uint32_t formatCount;
	vk::Result res;
	res = _gpus[0].getSurfaceFormatsKHR(_surface, &formatCount, nullptr);
	assert(res == vk::Result::eSuccess);
	std::vector<vk::SurfaceFormatKHR> surfaceFormats;
	surfaceFormats.resize(formatCount);
	res = _gpus[0].getSurfaceFormatsKHR(_surface, &formatCount, surfaceFormats.data());

	// if no format supported, fall back to a default one
	if (formatCount == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
	{
		_format = vk::Format::eB8G8R8A8Unorm;
	}
	else
	{
		assert(formatCount >= 1);
		_format = surfaceFormats[0].format;
	}

	// Surface capabilities and present mode count
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	res = _gpus[0].getSurfaceCapabilitiesKHR(_surface, &surfaceCapabilities);
	assert(res == vk::Result::eSuccess);
	uint32_t present_mode_count;
	res = (vk::Result)vkGetPhysicalDeviceSurfacePresentModesKHR(_gpus[0], _surface, &present_mode_count, nullptr);
	assert(res == vk::Result::eSuccess);
	std::vector<vk::PresentModeKHR> presentModes;
	presentModes.resize(present_mode_count);
	res = (vk::Result)vkGetPhysicalDeviceSurfacePresentModesKHR(_gpus[0], _surface, &present_mode_count, (VkPresentModeKHR*)presentModes.data());
	assert(res == vk::Result::eSuccess);

	vk::Extent2D swapchainExtent;
	// case undefined height or width
	if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
	{
		swapchainExtent.width = _window.resolution().width;
		swapchainExtent.height = _window.resolution().height;
		if (swapchainExtent.width > surfaceCapabilities.maxImageExtent.width)
		{
			swapchainExtent.width = surfaceCapabilities.maxImageExtent.width;
		}
		else if (swapchainExtent.width < surfaceCapabilities.minImageExtent.width)
		{
			swapchainExtent.width = surfaceCapabilities.minImageExtent.width;
		}
		if (swapchainExtent.height > surfaceCapabilities.maxImageExtent.height)
		{
			swapchainExtent.height = surfaceCapabilities.maxImageExtent.height;
		}
		else if (swapchainExtent.height < surfaceCapabilities.minImageExtent.height)
		{
			swapchainExtent.height = surfaceCapabilities.minImageExtent.height;
		}
	}
	else
	{
		// case ok
		swapchainExtent = surfaceCapabilities.currentExtent;
	}
	// FIFO present mode
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

	// Ensure we have 1 image before showing the next one
	uint32_t numDesiredSwapchainImages = surfaceCapabilities.minImageCount;
	vk::SurfaceTransformFlagBitsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
	{
		preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else
	{
		preTransform = surfaceCapabilities.currentTransform;
	}

	// Find supported alpha mode
	vk::CompositeAlphaFlagBitsKHR compositeAlpha;
	vk::CompositeAlphaFlagBitsKHR compositeAlphaFlags[] =
	{
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
		vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
		vk::CompositeAlphaFlagBitsKHR::eInherit
	};

	for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); ++i)
	{
		if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
		{
			compositeAlpha = compositeAlphaFlags[i];
			break;
		}
	}

	// Swapchain creation
	vk::SwapchainCreateInfoKHR swapchain_createInfo;
	swapchain_createInfo.setPNext(nullptr)
		.setSurface(_surface)
		.setMinImageCount(numDesiredSwapchainImages)
		.setImageFormat(_format)
		.setImageExtent(swapchainExtent)
		.setPreTransform(preTransform)
		.setCompositeAlpha(compositeAlpha)
		.setImageArrayLayers(1)
		.setPresentMode(presentMode)
		.setOldSwapchain(vk::SwapchainKHR(nullptr))
		.setClipped(true)
		.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr);

	uint32_t familyIndices[] = { _graphics_queue_family_index, _present_queue_family_index };
	if (_graphics_queue_family_index != _present_queue_family_index)
	{
		swapchain_createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(2)
			.setPQueueFamilyIndices(familyIndices);
	}

	res = _device.createSwapchainKHR(&swapchain_createInfo, nullptr, &_swapchain);
	assert(res == vk::Result::eSuccess);
	res = _device.getSwapchainImagesKHR(_swapchain, &_swapchain_image_count, nullptr);
	assert(res == vk::Result::eSuccess);
	std::vector<vk::Image> swapchainImages;
	swapchainImages.resize(_swapchain_image_count);
	res = _device.getSwapchainImagesKHR(_swapchain, &_swapchain_image_count, swapchainImages.data());
	assert(res == vk::Result::eSuccess);

	_buffers.resize(_swapchain_image_count);

	// Create image views info	
	vk::ImageSubresourceRange subresource_range;
	subresource_range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	vk::ImageViewCreateInfo imageView_createInfo;
	imageView_createInfo.setPNext(nullptr)
		.setFlags(vk::ImageViewCreateFlags())
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(_format)
		.setComponents(vk::ComponentSwizzle())
		.setSubresourceRange(subresource_range);

	for (uint32_t i = 0; i < _swapchain_image_count; ++i)
	{
		// Add images to buffer
		_buffers[i].image = swapchainImages[i];

		// Create and add views to buffer
		imageView_createInfo.setImage(swapchainImages[i]);
		res = _device.createImageView(&imageView_createInfo, nullptr, &_buffers[i].view);
		assert(res == vk::Result::eSuccess);
	}
}

void VkGraphics::vkInitDepthBuffer()
{
	vk::ImageCreateInfo image_ci;
	const vk::Format depth_format = vk::Format::eD16Unorm;
	vk::FormatProperties formatProperties;
	_gpus[0].getFormatProperties(depth_format, &formatProperties);
	if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
	{
		image_ci.setTiling(vk::ImageTiling::eLinear);
	}
	else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
	{
		image_ci.setTiling(vk::ImageTiling::eOptimal);
	}
	else
	{
		std::cout << "Format D16 unorm not supported" << std::endl;
		exit(-1);
	}

	// Buffer creation
	vk::Extent3D bufferExtent;
	bufferExtent.setWidth(_window.resolution().width)
		.setHeight(_window.resolution().height)
		.setDepth(1);
	image_ci.setPNext(nullptr)
		.setImageType(vk::ImageType::e2D)
		.setFormat(depth_format)
		.setExtent(bufferExtent)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setFlags(vk::ImageCreateFlags());

	vk::MemoryAllocateInfo memory_info;
	memory_info.setPNext(nullptr)
		.setAllocationSize(0)
		.setMemoryTypeIndex(0);

	vk::ImageSubresourceRange range;
	range.setAspectMask(vk::ImageAspectFlagBits::eDepth)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	vk::ImageViewCreateInfo iv_ci;
	iv_ci.setPNext(nullptr)
		.setImage(nullptr)
		.setFormat(depth_format)
		.setComponents(vk::ComponentSwizzle())
		.setSubresourceRange(range)
		.setViewType(vk::ImageViewType::e2D)
		.setFlags(vk::ImageViewCreateFlags());

	vk::MemoryRequirements mem_requirements;
	_depth.format = depth_format;

	// Create image
	vk::Result res;
	res = _device.createImage(&image_ci, nullptr, &_depth.image);
	assert(res == vk::Result::eSuccess);
	_device.getImageMemoryRequirements(_depth.image, &mem_requirements);
	memory_info.setAllocationSize(mem_requirements.size);

	// Determine the type of memory required
	bool p = memoryTypeFromProperties(mem_requirements.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eDeviceLocal, &memory_info.memoryTypeIndex);
	assert(p);

	// Allocate memory
	res = _device.allocateMemory(&memory_info, nullptr, &_depth.deviceMemory);
	assert(res == vk::Result::eSuccess);

	// Bind memory
	_device.bindImageMemory(_depth.image, _depth.deviceMemory, 0);

	// Create image view
	iv_ci.setImage(_depth.image);
	res = _device.createImageView(&iv_ci, nullptr, &_depth.imageView);
	assert(res == vk::Result::eSuccess);
}

void VkGraphics::vkInitUniformBuffer()
{
	// Creating base rendering and camera info
	Camera camera;
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 clip;
	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	view = glm::lookAt(
		glm::vec3(-5, 3, 10), // Camera position
		glm::vec3(0, 0, 0), // Camera looks at the origin
		glm::vec3(0, 1, 0) // Head is up
	);
	model = glm::mat4(1.0f);

	// Vulkan clip: Y (not) inverted and z halved
	clip = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f
	);

	camera.model(model);
	camera.view(view);
	camera.projection(projection);
	camera.clip(clip);
	_camera = camera;

	// Buffer creation
	vk::BufferCreateInfo buf_info;
	buf_info.setPNext(nullptr)
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.setSize(sizeof(_camera.MVP()))
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setFlags(vk::BufferCreateFlags());

	vk::Result res;
	res = _device.createBuffer(&buf_info, nullptr, &_uniform_data.buffer);
	assert(res == vk::Result::eSuccess);

	vk::MemoryRequirements mem_reqs;
	_device.getBufferMemoryRequirements(_uniform_data.buffer, &mem_reqs);

	vk::MemoryAllocateInfo alloc_info;
	alloc_info.setPNext(nullptr)
		.setAllocationSize(mem_reqs.size)
		.setMemoryTypeIndex(0);

	bool pass = memoryTypeFromProperties(mem_reqs.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		&alloc_info.memoryTypeIndex);
	assert(pass && "No mappable and coherent memory");
	res = _device.allocateMemory(&alloc_info, nullptr, &_uniform_data.memory);
	assert(res == vk::Result::eSuccess);

	uint8_t* pData;
	res = _device.mapMemory(_uniform_data.memory, 0, mem_reqs.size, vk::MemoryMapFlags(), (void **)&pData);
	assert(res == vk::Result::eSuccess);

	memcpy(pData, &_camera.MVP(), sizeof(_camera.MVP()));

	_device.unmapMemory(_uniform_data.memory);
	try {
		_device.bindBufferMemory(_uniform_data.buffer, _uniform_data.memory, 0);
	}
	catch (std::exception &e) {
		std::cout << "Impossible binding buffer memory: " << e.what() << std::endl;
		exit(-1);
	}

	_uniform_data.info.setBuffer(_uniform_data.buffer)
		.setOffset(0)
		.setRange(sizeof(_camera.MVP()));
}

void VkGraphics::vkInitPipelineLayout()
{
	vk::DescriptorSetLayoutBinding layout_binding;
	layout_binding.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutCreateInfo layout_ci;
	layout_ci.setPNext(nullptr)
		.setBindingCount(1)
		.setPBindings(&layout_binding);

	_desc_layout.resize(NUM_DESC_SETS);
	vk::Result res;
	for (uint32_t i = 0; i < NUM_DESC_SETS; ++i)
	{
		res = _device.createDescriptorSetLayout(&layout_ci, nullptr, &_desc_layout[i]);
		assert(res == vk::Result::eSuccess);
	}

	// Using the descriptor layouts to create a pipeline layout
	vk::PipelineLayoutCreateInfo pip_layout_ci;
	pip_layout_ci.setPNext(nullptr)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(nullptr)
		.setSetLayoutCount(NUM_DESC_SETS)
		.setPSetLayouts(_desc_layout.data());

	res = _device.createPipelineLayout(&pip_layout_ci, nullptr, &_pipeline_layout);
	assert(res == vk::Result::eSuccess);
}

void VkGraphics::vkInitDescriptorSet()
{
	vk::DescriptorPoolSize type_count[1];
	type_count[0].setDescriptorCount(NUM_DESC_SETS);

	vk::DescriptorPoolCreateInfo descriptor_pool_info;
	descriptor_pool_info.setPNext(nullptr)
		.setMaxSets(NUM_DESC_SETS)
		.setPoolSizeCount(NUM_DESC_SETS)
		.setPPoolSizes(type_count);

	vk::Result res;
	res = _device.createDescriptorPool(&descriptor_pool_info, nullptr, &_descriptor_pool);
	assert(res == vk::Result::eSuccess);

	vk::DescriptorSetAllocateInfo desc_alloc_info[1];
	desc_alloc_info[0].setPNext(nullptr)
		.setDescriptorPool(_descriptor_pool)
		.setDescriptorSetCount(NUM_DESC_SETS)
		.setPSetLayouts(_desc_layout.data());

	_descriptor_sets.resize(NUM_DESC_SETS);
	res = _device.allocateDescriptorSets(desc_alloc_info, _descriptor_sets.data());
	assert(res == vk::Result::eSuccess);

	vk::WriteDescriptorSet writes[1];
	
	writes[0].setPNext(nullptr)
		.setDstSet(_descriptor_sets[0])
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setPBufferInfo(&_uniform_data.info)
		.setDstArrayElement(0)
		.setDstBinding(0);

	_device.updateDescriptorSets(1, writes, 0, nullptr);
		
}

void VkGraphics::vkCreateRenderPass()
{
	vk::Semaphore imageAcquiredSemaphore;
	vk::SemaphoreCreateInfo ias_ci;
	ias_ci.setPNext(nullptr)
		.setFlags(vk::SemaphoreCreateFlags());

	vk::Result res;
	res = _device.createSemaphore(&ias_ci, nullptr, &imageAcquiredSemaphore);
	assert(res == vk::Result::eSuccess);

	// Acquiring swapchain to set layout
	res = _device.acquireNextImageKHR(_swapchain, UINT64_MAX, imageAcquiredSemaphore, 
		nullptr, &_current_buffer);
	assert(res >= vk::Result::eSuccess);

	_image_acquired_semaphore = imageAcquiredSemaphore;

	vk::AttachmentDescription attachments[2];
	attachments[0].setFormat(_format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
		.setFlags(vk::AttachmentDescriptionFlags());

	attachments[1].setFormat(_format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setFlags(vk::AttachmentDescriptionFlags());

	vk::AttachmentReference color_reference;
	color_reference.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depth_reference;
	depth_reference.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass;
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setFlags(vk::SubpassDescriptionFlags())
		.setInputAttachmentCount(0)
		.setPInputAttachments(nullptr)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_reference)
		.setPResolveAttachments(nullptr)
		.setPDepthStencilAttachment(&depth_reference)
		.setPreserveAttachmentCount(0)
		.setPPreserveAttachments(nullptr);

	vk::RenderPassCreateInfo rp_ci;
	rp_ci.setPNext(nullptr)
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(0)
		.setPDependencies(nullptr);

	res = _device.createRenderPass(&rp_ci, nullptr, &_render_pass);
	assert(res == vk::Result::eSuccess);
}

void VkGraphics::vkDestroySwapchain()
{
	for (uint32_t i = 0; i < _swapchain_image_count; ++i)
	{
		_device.destroyImageView(_buffers[i].view, nullptr);
	}
	_device.destroySwapchainKHR(_swapchain, nullptr);
}

void VkGraphics::onResize()
{
	vkDestroySwapchain();
	vkCreateSwapchain();
}

void VkGraphics::init()
{
	// Vulkan init
	vkInit();
	// Enumerate devices and select the first one
	vkEnumerateDevices();
	// Device creation
	vkCreateDevice();
	// Swapchain creation
	vkCreateSwapchain();
	// Command buffer creation
	vkCreateCommandBuffer();
	// Depth buffer init
	vkInitDepthBuffer();
	// Uniform buffer init
	vkInitUniformBuffer();
	// Pipeline layout init
	vkInitPipelineLayout();
	// Descriptor set initialization
	vkInitDescriptorSet();
	// Render pass creation
	vkCreateRenderPass();

	// Shader try
	_base_vertex_shader = ShaderFactory::loadVertexShader(_device, "./Shaders/vertex.spv");
}

void VkGraphics::destroy()
{
	// Destroy shader
	ShaderFactory::destroyShader(_device, _base_vertex_shader);

	// Clean up.
	_device.destroyRenderPass(_render_pass, nullptr);
	_device.destroySemaphore(_image_acquired_semaphore, nullptr);

	_device.destroyDescriptorPool(_descriptor_pool, nullptr);
	for (uint32_t i = 0; i < NUM_DESC_SETS; ++i)
	{
		_device.destroyDescriptorSetLayout(_desc_layout[i], nullptr);
	}
	_device.destroyPipelineLayout(_pipeline_layout, nullptr);
	_device.destroyBuffer(_uniform_data.buffer, nullptr);
	_device.freeMemory(_uniform_data.memory);
	_device.destroyImageView(_depth.imageView, nullptr);
	_device.destroyImage(_depth.image, nullptr);
	_device.freeMemory(_depth.deviceMemory, nullptr);
	vkDestroySwapchain();
	_device.freeCommandBuffers(_command_pool, _command_buffer);
	_device.destroyCommandPool(_command_pool, nullptr);
	_device.destroy(nullptr);
	_instance.destroySurfaceKHR(_surface);
	_instance.destroy();
}

std::vector<vk::ExtensionProperties> VkGraphics::getExtensionsProperties()
{
	uint32_t extensionCount;
	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<vk::ExtensionProperties> extensions(extensionCount);
	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
#ifdef _DEBUG
	for (auto & extension : extensions) {
		std::cout << extension.extensionName << std::endl;
	}
	return extensions;
#endif
}

bool VkGraphics::memoryTypeFromProperties(uint32_t typeBits, vk::MemoryPropertyFlags requirement_mask, uint32_t * typeIndex)
{
	// Search memtypes
	for (uint32_t i = 0; i < _memory_properties.memoryTypeCount; ++i)
	{
		if ((typeBits & 1) == 1)
		{
			// Memtype available
			if ((_memory_properties.memoryTypes[i].propertyFlags & requirement_mask) == requirement_mask)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}