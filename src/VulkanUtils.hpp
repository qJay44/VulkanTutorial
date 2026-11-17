#pragma once

#define CHECK_VK_RESULT(res, msg) \
  if (res != VK_SUCCESS) \
    error("\n[CHECK_VK_RESULT] Error in [{}: [{}] - msg: \n[\n{}\n\n], code {}\n", __FILE__, __LINE__, msg, (s32)res); \
