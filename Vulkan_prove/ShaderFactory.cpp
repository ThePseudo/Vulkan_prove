#include "ShaderFactory.h"

vk::ShaderModule ShaderFactory::loadVertexShader(vk::Device &device, string inputFile)
{
	vector<char> shaderText = readShader(inputFile);

	vk::PipelineShaderStageCreateInfo vertex_shader_info;
	vertex_shader_info.setPNext(nullptr)
		.setPSpecializationInfo(nullptr)
		.setFlags(vk::PipelineShaderStageCreateFlags())
		.setStage(vk::ShaderStageFlagBits::eVertex)
		.setPName("main");

	vk::ShaderModuleCreateInfo vertex_shader_module_info;
	vertex_shader_module_info
		.setPNext(nullptr)
		.setCodeSize(shaderText.size())
		.setPCode(reinterpret_cast<uint32_t*>(shaderText.data()))
		.setFlags(vk::ShaderModuleCreateFlags());

	vk::Result res = device.createShaderModule(&vertex_shader_module_info, nullptr, &vertex_shader_info.module);
	assert(res == vk::Result::eSuccess);
	return vertex_shader_info.module;
}

void ShaderFactory::destroyShader(vk::Device & device, vk::ShaderModule &shader)
{
	device.destroyShaderModule(shader, nullptr);
}

vector<char> ShaderFactory::readShader(string inputFile)
{
	vector<char> output;
	ifstream fin = ifstream(inputFile, ios::ate | ios::binary);
	if (!fin.is_open()) {
		exit(-1);
	}
	size_t nchars = (size_t)fin.tellg();
	fin.seekg(0);
	output.resize(nchars);
	fin.read(output.data(), nchars);
	fin.close();
	return output;
}