#pragma once

#include "utils/types.hpp"

namespace lve {

class LvePipeline {
public:
  LvePipeline(const fspath& vertPath, const fspath& fragPath);

private:
  void createGraphicsPipeline(fspath vertPath, fspath fragPath);
};

}; // namespace lve

