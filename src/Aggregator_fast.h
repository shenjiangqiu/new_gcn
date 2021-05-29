//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_AGGREGATOR_fast_H
#define GCN_SIM_AGGREGATOR_fast_H

#include "buffer_fast.h"

#include "fast_sched.h"
#include <unordered_map>
#include <vector>
namespace fast_sched {
class Aggregator_fast {
public:
  [[nodiscard]] bool isWorking() const { return working; }
  // TODO read buffer have latency
  void cycle();

  explicit Aggregator_fast(int totalCores);

  void add_task(const shared_ptr<Req> &req, unsigned node_size);
  unsigned get_total_rounds() const{
    return total_rounds;
  }
  unsigned get_total_operations() const{
    return total_operations;
  }
private:
  //
  int total_cores;
  bool working{false};
  uint64_t remaining_cycle{0};
  unsigned total_operations=0;
  unsigned total_rounds=0;
};
} // namespace fast_sched
#endif // GCN_SIM_AGGREGATOR_fast_H
