//
// Created by Jiangqiu shen on 1/15/21.
//

#ifndef GCN_SIM_DRAM_WRAPPER_H
#define GCN_SIM_DRAM_WRAPPER_H
#include "types.h"
#include <iostream>
class dram_wrapper {
public:
  virtual void send(uint64_t addr, bool is_write) = 0;
  [[nodiscard]] virtual bool available() const = 0;
  virtual void cycle() = 0;

  [[nodiscard]] virtual bool return_available() const = 0;

  virtual uint64_t pop() = 0;
  [[nodiscard]] virtual uint64_t get() const = 0;
  virtual ~dram_wrapper() = default;
  
  u_int64_t my_cycles;
  u_int64_t active_cycles;
  u_int64_t inflight_req_cnt;
  u_int64_t sum_inflight_req;
  u_int64_t  finished_read_req;
  u_int64_t  finished_write_req ;
};

#endif // GCN_SIM_DRAM_WRAPPER_H
