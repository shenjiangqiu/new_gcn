#include "globals.h"
#include <memory_interface.h>
#include <spdlog/spdlog.h>

struct set_level_struct {
  set_level_struct() {
    l = spdlog::get_level();
    spdlog::set_level(spdlog::level::debug);
  }
  ~set_level_struct() { spdlog::set_level(l); }
  spdlog::level::level_enum l;
};
void memory_interface::cycle() {
  //auto level= set_level_struct();
  m_mem->cycle();

  // send policy changed here
  if (!req_queue.empty()) {
    auto next_req = req_queue.front();
    assert(next_req->len > 0);
    bool already_send = addr_to_req_map.count(next_req->get_addr()) > 0;
    // write request will come back immediately when sent, so no need record
    if (next_req->req_type == mem_request::read) {
      spdlog::debug("memory_interface::cycle,from req_queue to addr_map: "
                    "addr:{},already_map:{}",
                    next_req->get_addr(), already_send);
      addr_to_req_map[next_req->get_addr()].push_back(next_req);
    }
    // addr, is_write
    if (already_send and next_req->req_type == mem_request::write) {
      throw "can't happen!";
    }
    if (!already_send) {

      spdlog::debug("memory_interface::cycle,from req_queue to outsendqueue: "
                    "addr:{},already_map:{}",
                    next_req->get_addr(), already_send);
      out_send_queue.push(
          {next_req->get_addr(), next_req->req_type == mem_request::write});
    }
    // calculate the input buffer traffic
    if (next_req->t == device_types::input_buffer) {
      global_definitions.total_read_input_traffic +=
          next_req->len > 64 ? 64 : next_req->len;

    } else if (next_req->t == device_types::edge_buffer) {
      global_definitions.total_read_edge_traffic +=
          next_req->len > 64 ? 64 : next_req->len;
    }
    if (int(next_req->len - 64) <= 0) {
      req_queue.pop();
      if (next_req->req_type == mem_request::write) {
        task_return_queue.push(next_req);
      }
    } else {
      next_req->add64();
      next_req->len -= 64;
    }
  }

  if (!response_queue.empty()) {
    auto &resp = response_queue.front();
    auto reqs = addr_to_req_map.at(resp);
    addr_to_req_map.erase(resp);
    for (auto &req : reqs) {

      spdlog::debug("memory_interface::cycle,from response to out: "
                    "id:{} addr:{},current_numbers:{}",
                    req->id, req->get_addr(), id_to_numreqs_map.at(req->id));
      assert(id_to_numreqs_map.at(req->id) > 0);
      if (--id_to_numreqs_map.at(req->id) == 0) {
        spdlog::debug("finished one task");
        id_to_numreqs_map.erase(req->id);
        task_return_queue.push(req);
      }
    }
    response_queue.pop();
  }

  // to dram
  int i = 0;
  // send 8 requests in a cycle
  for (i = 0; i < 8; i++) {
    if (!out_send_queue.empty() and
        m_mem->available(out_send_queue.front().first)) {
      auto req = out_send_queue.front();
      out_send_queue.pop();
      // addr, is_write
      m_mem->send(req.first, req.second);
    }
  }
  while (m_mem->return_available()) {
    auto req = m_mem->pop();
    response_queue.push(req);
  }
}

void memory_interface::send(std::shared_ptr<Req> req) {
  assert(req_queue.size() < waiting_size);
  if (req->get_addr() % 64 != 0) {
    throw "addr must be multiple of 64";
  }
  req_queue.push(req);
  auto num_reqs = (req->len + 63) / 64;
  // write request will not be back again, so no need to record.
  spdlog::debug("mem_interface::send,addr={},number={}", req->get_addr(),
                num_reqs);
  if (req->req_type == mem_request::read)
    id_to_numreqs_map.insert({req->id, num_reqs});
}

memory_interface::memory_interface(const std::string &dram_config_name,
                                   const std::string &dev_config_name,
                                   unsigned int waitingSize) {

  waiting_size = waitingSize;
  std::string mem_simulator = std::string(config::mem_sim);

  std::cout << "mem_simulator " << mem_simulator << "\n";

  if (mem_simulator.compare("ramulator") == 0)
    m_mem.reset(new ramulator_wrapper(dram_config_name, 64));
#ifdef USEDRAM3
  else if (mem_simulator.compare("dramsim3") == 0)
    m_mem.reset(new dramsim_wrapper(dram_config_name));
#endif

  else if (mem_simulator.compare("dramsim2") == 0)
    m_mem =
        std::make_shared<dramsim2_wrapper>(dram_config_name, dev_config_name);
  else {
    spdlog::error("fail to find the dram:{}", mem_simulator);
    throw;
    m_mem.reset(new ramulator_wrapper(dram_config_name, 64));
  }
}
