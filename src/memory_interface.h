#ifndef MEMORY_INTERFACE_H
#define MEMORY_INTERFACE_H

#include "dram_wrapper.h"
#include "dramsim_wrapper.h"
#include "dramsim2_wrapper.h"
#include <assert.h>
#include <map>
#include <memory>
#include <queue>
#include <ramulator_wrapper.h>
#include <types.h>
#include "globals.h"
class memory_interface {
private:
  unsigned waiting_size;
  std::queue<std::shared_ptr<Req>> req_queue;
  // addr, is_write
  std::queue<std::pair<unsigned long long, bool>> out_send_queue;
  std::queue<unsigned long long> response_queue;
  std::queue<std::shared_ptr<Req>> task_return_queue;

  std::map<unsigned, unsigned> id_to_numreqs_map;
  std::map<unsigned long long, std::shared_ptr<Req>> addr_to_req_map;
  std::shared_ptr<dram_wrapper> m_mem;
  double mem_tCK,cpu_tCK; 
  unsigned long  cycles;
  uint64_t dramPsPerClk, cpuPsPerClk;
  uint64_t dramPs, cpuPs;


public:
  bool empty() {
    return req_queue.empty() and out_send_queue.empty() and
           response_queue.empty() and task_return_queue.empty() and
           id_to_numreqs_map.empty() and addr_to_req_map.empty();
  }

  bool available() { return req_queue.size() < waiting_size; }

  void send(std::shared_ptr<Req> req);

  bool ret_available() { return task_return_queue.size() > 0; }

  std::shared_ptr<Req> get_req() {
    auto ret = task_return_queue.front();
    task_return_queue.pop();
    return ret;
  }

  memory_interface(const std::string &dram_config_name, 
                  const std::string &dev_config_name, unsigned int waitingSize);

  virtual ~memory_interface() = default;
  void cycle();
  void set_CLKs( double mem_CLK, double cpu_CLK);
};

#endif /* MEMORY_INTERFACE_H */
