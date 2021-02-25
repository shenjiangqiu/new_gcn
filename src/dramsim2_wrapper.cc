//
// Created by Jianhui on 2/24/2021.
//

#include "dramsim2_wrapper.h"
#include "spdlog/spdlog.h"

void dramsim2_wrapper::send(uint64_t addr, bool is_write) {
  if (is_write) {
    write_queue.push(addr);

  } else {
    read_queue.push(addr);
  }
}

bool dramsim2_wrapper::available() const {
  return read_queue.size() < 16 and write_queue.size() < 16;
}

void dramsim2_wrapper::cycle() {
  
  m_memory_system->update();

  if (!read_queue.empty()) {
    auto &&next = read_queue.front();
    if (m_memory_system->willAcceptTransaction( next )) {
      m_memory_system->addTransaction(false, next);
      read_queue.pop();
    }
  } else if (!write_queue.empty()) {
    auto &&next = write_queue.front();
    if (m_memory_system->willAcceptTransaction( next)) {
      m_memory_system->addTransaction(true, next);
      write_queue.pop();
    }
  }
}

bool dramsim2_wrapper::return_available() const { return !read_ret.empty(); }

uint64_t dramsim2_wrapper::pop() {
  auto addr = read_ret.front();
  read_ret.pop();
  return addr;
}

uint64_t dramsim2_wrapper::get() const { return read_ret.front(); }

dramsim2_wrapper::dramsim2_wrapper(const std::string& config_file,
                    const std::string& system_file) {
     
    m_memory_system =  DRAMSim::getMemorySystemInstance(config_file, system_file,
                                                  ".",8192);
  
    DRAMSim::TransactionCompleteCB *read_cb = new DRAMSim::Callback<dramsim2_wrapper, void, unsigned, uint64_t, uint64_t>(this, &dramsim2_wrapper::receive_read);
    DRAMSim::TransactionCompleteCB *write_cb = new DRAMSim::Callback<dramsim2_wrapper, void, unsigned, uint64_t, uint64_t>(this, &dramsim2_wrapper::receive_write);
    //m_memory_system->RegisterCallbacks(receive_read, receive_write, NULL);
    m_memory_system->RegisterCallbacks(read_cb, write_cb, NULL);


     spdlog::info("init dramsim");
}

dramsim2_wrapper::~dramsim2_wrapper() { 
   m_memory_system->printStats(true);//Yue
   delete m_memory_system; 
}

//DRAMSimMemory::DRAM_read_return_cb(uint32_t id, uint64_t addr, uint64_t memCycle)
void dramsim2_wrapper::receive_read(uint32_t id, uint64_t addr, uint64_t memCycle) { 
    read_ret.push(addr); 
}

void dramsim2_wrapper::receive_write(uint32_t id, uint64_t addr, uint64_t memCycle) {
  // do nothing
}
