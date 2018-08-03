#include "ShaderFactory.h"

/*
__forceinline vk::ShaderModule ShaderFactory::loadVertexShader(vk::Device &device, string inputFile)
{
	return createBasicShader(device, shader_type::tVertexShader, inputFile);
}

__forceinline vk::ShaderModule ShaderFactory::loadFragmentShader(vk::Device & device, string inputfile)
{
	return createBasicShader(device, shader_type::tFragmentShader, inputfile);
}*/

vk::ShaderModule ShaderFactory::loadVertexShader(vk::Device & device, string inputFile)
{
	return createBasicShader(device, shader_type::tVertexShader, inputFile);
}

vk::ShaderModule ShaderFactory::loadFragmentShader(vk::Device & device, string inputfile)
{
	return createBasicShader(device, shader_type::tFragmentShader, inputfile);
}

void ShaderFactory::destroyShader(vk::Device & device, vk::ShaderModule &shader)
{
	device.destroyShaderModule(shader, nullptr);
}

vk::ShaderModule ShaderFactory::createBasicShader(vk::Device & device, shader_type type, string inputFile)
{
	vector<char> shaderText = readShader(inputFile);
	vk::PipelineShaderStageCreateInfo shader_info;
	shader_info.setPNext(nullptr)
		.setPSpecializationInfo(nullptr)
		.setFlags(vk::PipelineShaderStageCreateFlags())
		.setPName("main");
		shader_info.setStage(static_cast<vk::ShaderStageFlagBits>(type));

	vk::ShaderModuleCreateInfo shader_module_info;
	shader_module_info
		.setPNext(nullptr)
		.setCodeSize(shaderText.size())
		.setPCode(reinterpret_cast<uint32_t*>(shaderText.data()))
		.setFlags(vk::ShaderModuleCreateFlags());

	vk::Result res = device.createShaderModule(&shader_module_info, nullptr, &shader_info.module);
	assert(res == vk::Result::eSuccess);
	return shader_info.module;
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