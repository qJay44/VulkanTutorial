## Sources
* https://www.youtube.com/playlist?list=PLA0dXqQjCx0RntJy1pqje9uHRF1Z5vZgA

###### Hand Coordinate System:
* Right (+x, +y, -z) - OpenGL
* Left  (+x, +y, +z) - Vulkan

###### Depth (Z):
* \[-1, 1] - OpenGL
* \[ 0, 1] - Vulkan

###### Shaders:
* `gl_VertexID`    - only in OpenGL
* `gl_VertexIndex` - only in Vulkan

## BRANCH (TUTORIAL) SPECIFIC NOTES:

###### ogldev:

* Creating VkInstance with hardcoded extensions (instead of quering them)
* Hardcoded Vulkan version (instead of quering latest available version)
* Selecting first available physical device (instead of using score system)

###### brendan-galea:

* Selecting first available physical device (instead of using score system)

