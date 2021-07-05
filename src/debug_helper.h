//
// Created by Jiangqiu shen on 5/11/21.
//

#ifndef GCN_SIM_DEBUG_HELPER_H
#define GCN_SIM_DEBUG_HELPER_H
#include "globals.h"
#include <spdlog/spdlog.h>
#define GCN_INFO(format, ...)                                                  \
  spdlog::info("{}:{} cycle:{}, " format, __FILE__, \
                                          __LINE__, global_definitions.cycle,  \
                                          __VA_ARGS__)
#define GCN_DEBUG(format, ...)                                                 \
  spdlog::debug(                                    \
      "{}:{} cycle:{}, " format, __FILE__, __LINE__, global_definitions.cycle, \
      __VA_ARGS__)
#define GCN_ERROR(format, ...)                                                 \
  spdlog::error(                                    \
      "{}:{} cycle:{}, " format, __FILE__, __LINE__, global_definitions.cycle, \
      __VA_ARGS__)

#define GCN_INFO_S(format)                                                     \
  spdlog::info("{}:{} cycle:{}, " format, __FILE__, \
                                          __LINE__, global_definitions.cycle)
#define GCN_DEBUG_S(format)                                                    \
  spdlog::debug(                                    \
      "{}:{} cycle:{}, " format, __FILE__, __LINE__, global_definitions.cycle)
#define GCN_ERROR_S(format)                                                    \
  spdlog::error(                                    \
      "{}:{} cycle:{}, " format, __FILE__, __LINE__, global_definitions.cycle)

#endif // GCN_SIM_DEBUG_HELPER_H
