#pragma once

#include <vector>
#include <fstream>
#include <vulkan/vulkan.hpp>

using namespace std;

class ShaderFactory
{
public:
	static vk::ShaderModule loadVertexShader(vk::Device &device, string inputFile);

	//Shader destructor
	static void destroyShader(vk::Device &device, vk::ShaderModule &shader);
private:
	static vector<char> readShader(string inputFile); // The file is supposed to be SPIR-V compiled
};

