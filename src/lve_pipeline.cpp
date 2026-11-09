#include "lve_pipeline.hpp"
#include "utils/utils.hpp"

namespace lve {

LvePipeline::LvePipeline(const fspath& vertPath, const fspath& fragPath) {
  createGraphicsPipeline(vertPath, fragPath);
}

void LvePipeline::createGraphicsPipeline(fspath vertPath, fspath fragPath) {
  vertPath = "Build/spv" / vertPath; vertPath += ".spv";
  fragPath = "Build/spv" / fragPath; fragPath += ".spv";

  auto vertCode = readFile(vertPath);
  auto fragCode = readFile(fragPath);

  std::println("Vert code size: {}", vertCode.size());
  std::println("Frag code size: {}", fragCode.size());
}

} // namespace lve

