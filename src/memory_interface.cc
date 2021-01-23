#include "globals.h"
#include <memory_interface.h>
void memory_interface::cycle() {

  m_ramulator->cycle();
  // send policy changed here
  if (!req_queue.empty()) {
    auto next_req = req_queue.front();
    if (next_req->len > 0) {
      // write request will come back immediately when sent, so no need record
      if (next_req->req_type == mem_request::read)
        addr_to_req_map.insert({next_req->addr, next_req});
      // addr, is_write
      out_send_queue.push(
          {next_req->addr, next_req->req_type == mem_request::write});
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
        next_req->addr += 64;
        next_req->len -= 64;
      }
    }
  }
  if (!response_queue.empty()) {
    auto &resp = response_queue.front();
    auto req = addr_to_req_map.at(resp);
    addr_to_req_map.erase(resp);
    if (--id_to_numreqs_map.at(req->id) == 0) {
      id_to_numreqs_map.erase(req->id);
      task_return_queue.push(req);
    }
    response_queue.pop();
  }

  // to dram
  if (!out_send_queue.empty() and m_ramulator->available()) {
    auto req = out_send_queue.front();
    out_send_queue.pop();
    // addr, is_write
    m_ramulator->send(req.first, req.second);
  }
  if (m_ramulator->return_available()) {
    auto req = m_ramulator->pop();
    response_queue.push(req);
  }
}
void memory_interface::send(std::shared_ptr<Req> req) {
  assert(req_queue.size() < waiting_size);
  req_queue.push(req);
  auto num_reqs = (req->len + 63) / 64;
  // write request will not be back again, so no need to record.
  if (req->req_type == mem_request::read)
    id_to_numreqs_map.insert({req->id, num_reqs});
}
