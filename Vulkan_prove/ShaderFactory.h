#pragma once

#include <vector>
#include <fstream>
#include <vulkan/vulkan.hpp>

using namespace std;

class ShaderFactory
{
	enum shader_type {
		tVertexShader = vk::ShaderStageFlagBits::eVertex,
		tFragmentShader = vk::ShaderStageFlagBits::eFragment,
		tComputeShader = vk::ShaderStageFlagBits::eCompute,
		tGeometry = vk::ShaderStageFlagBits::eGeometry,
		tTassellationControl = vk::ShaderStageFlagBits::eTessellationControl,
		tTassellationEvaluation = vk::ShaderStageFlagBits::eTessellationEvaluation,
		tAllGraphics = vk::ShaderStageFlagBits::eAllGraphics,
		tAll = vk::ShaderStageFlagBits::eAll
	};
public:
	static vk::ShaderModule loadVertexShader(vk::Device &device, string inputFile);
	static vk::ShaderModule loadFragmentShader(vk::Device &device, string inputfile);

	//Shader destructor
	static void destroyShader(vk::Device &device, vk::ShaderModule &shader);
private:
	static vk::ShaderModule createBasicShader(vk::Device &device, shader_type type, string inputFile);
	static vector<char> readShader(string inputFile); // The file is supposed to be SPIR-V compiled
};

