#include "VulkanApp.hpp"

namespace lve {

VulkanApp::~VulkanApp() {
  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.size()), m_cmdBufs.data());
}

void VulkanApp::init(const char* appName, GLFWwindow* window) {
  m_core.init(appName, window);
  m_numImages = m_core.getNumImages();
  createCommandBuffers();
}

void VulkanApp::render() {

}

void VulkanApp::createCommandBuffers() {
  m_cmdBufs.resize(m_numImages);
  m_core.createCommandBuffers(m_numImages, m_cmdBufs.data());
}

} // namespace lve

