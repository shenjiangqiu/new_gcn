#include "boost/range/irange.hpp"
#include "debug_helper.h"
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
  // auto level= set_level_struct();
  auto update_traffic = [](unsigned size, device_types type) {
    switch (type) {
    case device_types::input_buffer:
      global_definitions.total_read_traffic_input += size;
      global_definitions.total_read_traffic += size;
      break;
    case device_types::edge_buffer:
      global_definitions.total_read_traffic += size;
      global_definitions.total_read_traffic_edge += size;
      break;
    case device_types::output_buffer:
      global_definitions.total_write_traffic += size;
      break;

    default:
      throw;
    }
  };

  m_mem->cycle();

  // send policy changed here
  while (!req_queue.empty()) {
    auto &req = req_queue.front();
    // there are two cases:
    // 1, single addr with a len
    // 2, multiple addr

    // the function to insert a request
    //  insert to requst to mem, record the req info
    auto send_reqs = [&](uint64_t addr, std::shared_ptr<Req> &req) {
      // the addr is all new! send it to lower memroy
      if (!addr_to_req_map.count(addr)) {

        out_send_queue.push({addr, req->req_type == mem_request::write});
        // fix bug here
        // the traffic should only contain those really send to dram.
        // before(update every time call send_reqs!!)
        update_traffic(64, req->t);
      }

      // add the req to the record
      insert_pending_request(addr, req->id);
    };
    // handle write request here.
    if (req->req_type == mem_request::write) {
      GCN_DEBUG("{}:{} ,deal with write: {}", __FILE__, __LINE__, *req);
      // just send it and return
      while (m_mem->available(req->get_single_addr())) {
        auto &&len = req->get_len();
        auto &&addr = req->get_single_addr();
        if (len > 0) {
          GCN_DEBUG("{}:{} ,send write: {}", __FILE__, __LINE__, *req);

          m_mem->send(addr, true);
          // task_return_queue.push(req);
          // req_queue.pop();

          // the write traffic!
          assert(req->t == device_types::output_buffer);
          update_traffic(64, device_types::output_buffer);
          if (len <= 64) {
            GCN_DEBUG("{}:{} ,finished write: {}", __FILE__, __LINE__, *req);

            task_return_queue.push(req);
            req_queue.pop();
            return;
          }
          len -= 64;
          addr += 64;
          req->set_addr(addr, len);
        }
      }
      // do not finished this request, return and waiting further operation
      // the request is still in the queue.
      // fix bug here, I forgot to return ....
      return;
    }

    // now it's read because write return above

    if (req->is_single_addr()) {
      // it's a continues addr,like edge, output...,
      auto addr = req->get_single_addr();
      auto len = req->get_len();
      auto count = (len + 63) / 64;
      for (auto i : boost::irange(0u, count)) {
        send_reqs(addr + i * 64, req);
      }
    } else {
      auto &&addrs = req->get_addr();
      for (auto addr : addrs) {
        send_reqs(addr, req);
      }
    }

    req_queue.pop();
  }

  // handle the return from mem logic
  while (!response_queue.empty()) {
    auto &&ret = response_queue.front();
    GCN_DEBUG("finished: {}", ret);

    auto &&finished = receive_req(ret);
    GCN_DEBUG("finished req:[{}]", fmt::join(finished, ","));
    // find req by id
    for (auto &&id : finished) {
      task_return_queue.push(id_to_reqs_map.at(id));
      id_to_reqs_map.erase(id);
    }
    response_queue.pop();
  }

  // to dram
  // noticed here, inorder to simulate HBM multichannel behavior, we need to
  // try to send one request for each channel Fixed here, because the channel
  // logic is already implemented in the mem_wrapper, so here we just need to
  // send them all to the wrapper
  while (!out_send_queue.empty()) {
    auto &&addr_is_write = out_send_queue.front();
    if (m_mem->available(addr_is_write.first)) {
      m_mem->send(addr_is_write.first, addr_is_write.second);
      out_send_queue.pop();
    } else {
      break;
    }
  }

  // from dram
  while (m_mem->return_available()) {
    auto req = m_mem->pop();
    response_queue.push(req);
  }
}

void memory_interface::send(const std::shared_ptr<Req> &req) {
  assert(req_queue.size() < waiting_size);
  auto &&addrs = req->get_addr();
#ifdef DEBUG
  // check if the address are aligned by 64 bytes.
  if (std::any_of(addrs.begin(), addrs.end(),
                  [](auto addr) { return addr % 64 != 0; })) {
    throw std::runtime_error("addr must be multiple of 64");
  }
#endif
  // we just only need to record the read request, write request are return
  // immediately when scheduled.
  if (req->req_type == mem_request::read)
    id_to_reqs_map.insert({req->id, req});
  GCN_DEBUG("received req:{}", *req);
  req_queue.push(req);

  static uint64_t print_sign = 0;
  if (print_sign % 100 == 0) {
    GCN_INFO("mem_interface is still receiving request:{}", print_sign);
  }
  print_sign++;
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
    GCN_ERROR("fail to find the dram:{}", mem_simulator);
    throw;
    m_mem.reset(new ramulator_wrapper(dram_config_name, 64));
  }
}
