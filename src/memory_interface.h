#ifndef MEMORY_INTERFACE_H
#define MEMORY_INTERFACE_H

#include <assert.h>
#include <map>
#include <memory>
#include <queue>
#include <ramulator_wrapper.h>
#include <types.h>

class memory_interface {
private:
  unsigned waiting_size;
  std::queue<std::shared_ptr<Req>> req_queue;
  std::queue<std::pair<unsigned long long, bool>> out_send_queue;
  std::queue<unsigned long long> response_queue;
  std::queue<std::shared_ptr<Req>> task_return_queue;

  std::map<unsigned, unsigned> id_to_numreqs_map;
  std::map<unsigned long long, std::shared_ptr<Req>> addr_to_req_map;
  std::shared_ptr<ramulator_wrapper> m_ramulator;

public:
  bool empty() {
    return req_queue.empty() and out_send_queue.empty() and
           response_queue.empty() and task_return_queue.empty() and
           id_to_numreqs_map.empty() and addr_to_req_map.empty();
  }

  bool avaliable() { return req_queue.size() < waiting_size; }

  void send(std::shared_ptr<Req> req);

  bool ret_avaliable() { return task_return_queue.size() > 0; }

  std::shared_ptr<Req> get_req() {
    auto ret = task_return_queue.front();
    task_return_queue.pop();
    return ret;
  }

  memory_interface(const std::string &dram_config_name,
                   unsigned int waitingSize)
      : m_ramulator(new ramulator_wrapper(dram_config_name, 64)),
        waiting_size(waitingSize) {}

  void cycle();
};

#endif /* MEMORY_INTERFACE_H */
