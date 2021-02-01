//
// Created by Jiangqiu shen on 1/15/21.
//

#ifndef GCN_SIM_DRAM_WRAPPER_H
#define GCN_SIM_DRAM_WRAPPER_H
#include "types.h"
class dram_wrapper {
public:
  virtual void send(unsigned long long addr, bool is_write) = 0;
  [[nodiscard]] virtual bool available() const = 0;
  virtual void cycle() = 0;

  [[nodiscard]] virtual bool return_available() const = 0;

  virtual unsigned long long pop() = 0;
  [[nodiscard]] virtual unsigned long long get() const = 0;
  virtual ~dram_wrapper() = default;
};

#endif // GCN_SIM_DRAM_WRAPPER_H
