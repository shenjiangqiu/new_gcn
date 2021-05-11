#ifndef MEMORY_INTERFACE_H
#define MEMORY_INTERFACE_H

#include "dram_wrapper.h"
#include "dramsim2_wrapper.h"
#include "dramsim_wrapper.h"
#include "globals.h"
#include <assert.h>
#include <map>
#include <memory>
#include <queue>
#include <ramulator_wrapper.h>
#include <types.h>

using uint = unsigned;
using ull = uint64_t;
class memory_interface {
private:
  unsigned waiting_size;

  std::queue<std::shared_ptr<Req>> req_queue;
  // addr, is_write
  std::queue<std::pair<ull, bool>> out_send_queue;
  std::queue<uint64_t> response_queue;
  std::queue<std::shared_ptr<Req>> task_return_queue;

  std::map<unsigned, std::shared_ptr<Req>> id_to_reqs_map;
  std::shared_ptr<dram_wrapper> m_mem;

  std::map<uint, std::set<ull>> req_id_to_addr_map;
  std::map<ull, std::set<uint>> addr_to_req_map;

  // record the request, when the request returned, we can find all the id
  // related to this addr, and find all addrs related to this id to decide if to
  // return or not
  void insert_pending_request(ull addr, uint req_id) {
    req_id_to_addr_map[req_id].insert(addr);
    addr_to_req_map[addr].insert(req_id);
  }

  // the request returned from the lower memory
  std::vector<uint> receive_req(ull addr) {
    std::vector<uint> finished_reqs;
    // find all id related to this addr
    auto &id_set = addr_to_req_map[addr];

    // for each id, test if it's already finished, if yes, return the request
    // and delete the record
    for (auto id : id_set) {
      req_id_to_addr_map[id].erase(addr);
      if (req_id_to_addr_map[id].empty()) {
        finished_reqs.push_back(id);
        req_id_to_addr_map.erase(id);
      }
    }
    // the addr is retired
    addr_to_req_map.erase(addr);

    // return the all finished reqs
    return finished_reqs;
  }

public:
  bool empty() {
    return req_queue.empty() and out_send_queue.empty() and
           response_queue.empty() and task_return_queue.empty() and
           req_id_to_addr_map.empty() and addr_to_req_map.empty() and
           id_to_reqs_map.empty();
  }

  bool available() { return req_queue.size() < waiting_size; }

  void send(std::shared_ptr<Req> &req);

  bool ret_available() { return !task_return_queue.empty(); }

  std::shared_ptr<Req> get_req() {
    auto ret = task_return_queue.front();
    task_return_queue.pop();
    return ret;
  }
  memory_interface(const std::string &dram_config_name,
                   const std::string &dev_config_name,
                   unsigned int waitingSize);
  virtual ~memory_interface() = default;
  void cycle();
};

#endif /* MEMORY_INTERFACE_H */
