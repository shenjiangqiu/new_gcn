//
// Created by ivy22 on 2/1/2021.
//

#include "dramsim_wrapper.h"
void dramsim_wrapper::send(unsigned long long addr, bool is_write) {
  if (is_write) {
    write_queue.push(addr);

  } else {
    read_queue.push(addr);
  }
}
bool dramsim_wrapper::available() const {
  return read_queue.size() < 16 and write_queue.size() < 16;
}
void dramsim_wrapper::cycle() {
  if (!read_queue.empty()) {
    auto &&next = read_queue.front();
    if (m_memory_system->WillAcceptTransaction(next, false)) {
      m_memory_system->AddTransaction(next, false);
      read_queue.pop();
    }
  } else if (!write_queue.empty()) {
    auto &&next = write_queue.front();
    if (m_memory_system->WillAcceptTransaction(next, true)) {
      m_memory_system->AddTransaction(next, true);
      write_queue.pop();
    }
  }
}
bool dramsim_wrapper::return_available() const { return !read_ret.empty(); }
unsigned long long dramsim_wrapper::pop() {
  auto addr = read_ret.front();
  read_ret.pop();
  return addr;
}
unsigned long long dramsim_wrapper::get() const { return read_ret.front(); }
dramsim_wrapper::dramsim_wrapper(const std::string &config_file,
                                 const std::string &output_dir) {
  m_memory_system = dramsim3::GetMemorySystem(
      config_file, output_dir,
      [this](unsigned long long addr) { this->receive_read(addr); },
      [this](unsigned long long addr) { this->receive_write(addr); });
}
dramsim_wrapper::~dramsim_wrapper() { delete m_memory_system; }
void dramsim_wrapper::receive_read(unsigned long long int addr) {
  read_ret.push(addr);
}
void dramsim_wrapper::receive_write(unsigned long long int addr) {
  // do nothing
}
