//
// Created by ivy22 on 2/1/2021.
//
#ifdef USEDRAM3
#include "dramsim_wrapper.h"
#include "spdlog/spdlog.h"
void dramsim_wrapper::send(uint64_t addr, bool is_write) {
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
  m_memory_system->ClockTick();
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
uint64_t dramsim_wrapper::pop() {
  auto addr = read_ret.front();
  read_ret.pop();
  return addr;
}
uint64_t dramsim_wrapper::get() const { return read_ret.front(); }
dramsim_wrapper::dramsim_wrapper(const std::string &config_file,
                                 const std::string &output_dir) {
  m_memory_system = dramsim3::GetMemorySystem(
      config_file, output_dir,
      [this](uint64_t addr) { this->receive_read(addr); },
      [this](uint64_t addr) { this->receive_write(addr); });
  spdlog::info("init dramsim");
}
dramsim_wrapper::~dramsim_wrapper() { 
   m_memory_system->PrintStats();//Yue
   delete m_memory_system; }

void dramsim_wrapper::receive_read(uint64_t addr) { read_ret.push(addr); }
void dramsim_wrapper::receive_write(uint64_t addr) {
  // do nothing
}
#endif