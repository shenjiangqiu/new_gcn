//
// Created by Jiangqiu shen on 1/15/21.
//

#ifndef GCN_SIM_DRAM_WRAPPER_H
#define GCN_SIM_DRAM_WRAPPER_H
#include "types.h"
class dram_wrapper {
public:
  virtual void send(uint64_t addr, bool is_write) = 0;
  [[nodiscard]] virtual bool available() const = 0;
  virtual void cycle() = 0;

  [[nodiscard]] virtual bool return_available() const = 0;

  virtual uint64_t pop() = 0;
  [[nodiscard]] virtual uint64_t get() const = 0;
  virtual ~dram_wrapper() = default;
};

#endif // GCN_SIM_DRAM_WRAPPER_H
