#include "VulkanShader.hpp"

#include "glslang/Include/glslang_c_interface.h"
#include "glslang/Public/resource_limits_c.h"
#include "utils/clrp.hpp"

namespace lve {

namespace {

glslang_stage_t getStageFromFileExtension(const std::string& ext) {
  static const std::unordered_map<std::string, glslang_stage_t> mappedExtensions = {
    {".vert", GLSLANG_STAGE_VERTEX},
    {".frag", GLSLANG_STAGE_FRAGMENT},
    {".geom", GLSLANG_STAGE_GEOMETRY},
    {".comp", GLSLANG_STAGE_COMPUTE},
    {".tesc", GLSLANG_STAGE_TESSCONTROL},
    {".tese", GLSLANG_STAGE_TESSEVALUATION},
  };

  glslang_stage_t stage{};
  bool found = false;

  for (const auto& [e, s] : mappedExtensions) {
    if (e == ext) {
      found = true;
      stage = s;
      break;
    }
  }

  if (!found)
    error("[getStageFromFileExtension] Not supported extention [{}]", ext);

  return stage;
}

std::vector<u32> compile(VkDevice& device, glslang_stage_t stage, const std::string& shaderStr, VkShaderModule& shaderModule) {
  glslang_input_t input{};
  input.language = GLSLANG_SOURCE_GLSL;
  input.stage = stage;
  input.client = GLSLANG_CLIENT_OPENGL;
  input.client_version = GLSLANG_TARGET_VULKAN_1_1;
  input.target_language = GLSLANG_TARGET_SPV;
  input.target_language_version = GLSLANG_TARGET_SPV_1_3;
  input.code = shaderStr.c_str();
  input.default_version = 100;
  input.default_profile = GLSLANG_NO_PROFILE;
  input.force_default_version_and_profile = false;
  input.forward_compatible = false;
  input.messages = GLSLANG_MSG_DEFAULT_BIT;
  input.resource = glslang_default_resource();

  glslang_shader_t* shader = glslang_shader_create(&input);
  clrp::clrp_t clrp{.fg = clrp::FG::RED};

	if (!glslang_shader_preprocess(shader, &input))	{
    std::println("{}", clrp::format("GLSL preprocessing failed\n", clrp));
    std::println("{}", glslang_shader_get_info_log(shader));
    std::println("{}", glslang_shader_get_info_debug_log(shader));
    glslang_shader_delete(shader);
		return {};
	}

	if (!glslang_shader_parse(shader, &input)) {
    std::println("{}", clrp::format("GLSL parsing failed\n", clrp));
    std::println("{}", glslang_shader_get_info_log(shader));
    std::println("{}", glslang_shader_get_info_debug_log(shader));
    glslang_shader_delete(shader);
		return {};
	}

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
    std::println("{}", clrp::format("GLSL linking failed\n", clrp));
    std::println("{}", glslang_program_get_info_log(program));
    std::println("{}", glslang_program_get_info_debug_log(program));
    glslang_shader_delete(shader);
		return {};
	}

  glslang_program_SPIRV_generate(program, stage);
  std::vector<u32> spirvData(glslang_program_SPIRV_get_size(program));
  glslang_program_SPIRV_get(program, spirvData.data());

  const char* spirvMsg = glslang_program_SPIRV_get_messages(program);
  if (spirvMsg)
    std::println("SPIR-V message: [{}]", clrp::format(spirvMsg, clrp));

  VkShaderModuleCreateInfo shaderCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .codeSize = spirvData.size() * sizeof(u32),
    .pCode = (const u32*)spirvData.data(),
  };

  VkResult res = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &shaderModule);
  CHECK_VK_RESULT(res, "vkCreateShaderModule");

  glslang_program_delete(program);
  glslang_shader_delete(shader);

  return spirvData;
}

} // namespace

void VulkanShader::createShaderModuleFromBinary(VkDevice& device, const fspath& path) {
  std::string source = readFile(path);

  VkShaderModuleCreateInfo shaderCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .codeSize = source.size() * sizeof(char),
    .pCode = (const u32*)source.c_str(), // reinterpret_cast?
  };

  VkResult res = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &shaderModule);
  CHECK_VK_RESULT(res, "vkCreateShaderModule");
  dprint("Created shader from binary file: [{}]", path.string());
}

void VulkanShader::createShaderModuleFromText(VkDevice& device, const fspath& path) {
  std::string source = readFile(path);

  glslang_stage_t stage = getStageFromFileExtension(path.extension());

  glslang_initialize_process();

  std::vector<u32> spirvData = compile(device, stage, source, shaderModule);
  if (!spirvData.empty()) {
    dprint("Created .spv shader from text file: [{}]", path.string());
    fspath writePath = path;
    writePath += ".spv";
    writePath = writePath.parent_path() / "Build/spv" / writePath.filename();

    writeFile(writePath, spirvData.data(), spirvData.size() * sizeof(u32));
  } else {
    warning("[VulkanShader::compile] Compiled data size is 0");
  }

  glslang_finalize_process();
}

} // namespace lve

