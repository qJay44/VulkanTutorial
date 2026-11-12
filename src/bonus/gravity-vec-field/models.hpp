#pragma once

#include "../../lve_model.hpp"

namespace lve::bonus::gvf {

inline std::unique_ptr<LveModel> createSquareModel(LveDevice& device, vec2 offset) {
  std::vector<LveModel::Vertex> vertices = {
    {vec2{-0.5f, -0.5f} + offset},
    {vec2{ 0.5f,  0.5f} + offset},
    {vec2{-0.5f,  0.5f} + offset},
    {vec2{-0.5f, -0.5f} + offset},
    {vec2{ 0.5f, -0.5f} + offset},
    {vec2{ 0.5f,  0.5f} + offset},
  };

  return std::make_unique<LveModel>(device, vertices);
}

inline std::unique_ptr<LveModel> createCircleModel(LveDevice& device, int numSides) {
  std::vector<LveModel::Vertex> uniqueVertices{};
  for (int i = 0; i < numSides; i++) {
    float angle = i * glm::two_pi<float>() / numSides;
    uniqueVertices.push_back({{cos(angle), sin(angle)}});
  }

  uniqueVertices.push_back({{}}); // center

  std::vector<LveModel::Vertex> vertices{};
  for (int i = 0; i < numSides; i++) {
    vertices.push_back(uniqueVertices[i]);
    vertices.push_back(uniqueVertices[(i + 1) % numSides]);
    vertices.push_back(uniqueVertices[numSides]);
  }

  return std::make_unique<LveModel>(device, vertices);
}

}; // namespace lve::bonus::gvf

