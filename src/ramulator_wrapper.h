#ifndef RAMULATOR_WRAPPER_H
#define RAMULATOR_WRAPPER_H

#include "Cache.h"
#include "Memory.h"
#include "RamulatorConfig.h"
#include "Request.h"
#include "Statistics.h"
#include <cctype>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <map>
#include <queue>
#include <set>
#include <tuple>
#include "dram_wrapper.h"
class ramulator_wrapper: public dram_wrapper {
public:
  void send(uint64_t addr, bool is_write) override;
  bool available() const override { return in_queue.size() <= 64; }
  void tick() ;

  void finish();

  ramulator_wrapper(ramulator::Config configs, int cacheLine);

  ~ramulator_wrapper();

  void call_back(ramulator::Request &req);

  [[nodiscard]] bool empty() const;

  [[nodiscard]] std::string get_internal_size() const;

  [[nodiscard]] std::string get_line_trace() const;

  [[nodiscard]] uint64_t get() const override { return out_queue.front(); }

  uint64_t pop() override {
    auto ret = out_queue.front();
    out_queue.pop();
    return ret;
  }

  [[nodiscard]] bool return_available() const override { return !out_queue.empty(); }

  void cycle() override;
  
private:
  double tCK;
  unsigned long long outgoing_reqs = 0;
  //addr,iswrite
  std::queue<std::pair<uint64_t, bool>> in_queue;
  std::queue<uint64_t> out_queue;
  ramulator::MemoryBase *mem;
};

#endif