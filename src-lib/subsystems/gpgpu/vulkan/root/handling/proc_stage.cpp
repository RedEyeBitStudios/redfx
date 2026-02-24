#include <internals/subsystems/gpgpu/vulkan/processor_stage_resources.hpp>
#include <utils/file_utils.hpp>
#include <format>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStageResources_Vulkan;

VkShaderModule ClassImpl::makeShader(const GPGPU_Device_Vulkan::Commons& commons, std::string_view path)
{
	auto shader_code_buf = nxcraft::FileUtils::readWholeFile(std::format("shaders/vulkan/{}", path));
	
	VkShaderModule cache;
	const VkShaderModuleCreateInfo shader_info
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = shader_code_buf.size(),
		.pCode = reinterpret_cast<uint32_t*>(shader_code_buf.data())
	};
	vkCreateShaderModule(commons.dvc, &shader_info, nullptr, &cache);

	return cache;
}

void ClassImpl::prepareDescriptors(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data)
{

}
void ClassImpl::preparePipelines(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data)
{

}
void ClassImpl::preparePipelineLayouts(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data)
{
	
}