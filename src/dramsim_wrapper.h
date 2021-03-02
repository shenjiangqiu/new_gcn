//
// Created by ivy22 on 2/1/2021.
//

#ifndef GCN_SIM_DRAMSIM_WRAPPER_H
#define GCN_SIM_DRAMSIM_WRAPPER_H
#ifdef USEDRAM3
#include "dram_wrapper.h"
#include "dramsim3.h"
#include "queue"

class dramsim_wrapper : public dram_wrapper {
public:
  dramsim_wrapper(const std::string &config_file,
                  const std::string &output_dir=".");
  void send(uint64_t addr, bool is_write) override;
  bool available() const override;
  void cycle() override;
  bool return_available() const override;
  uint64_t pop() override;
  uint64_t get() const override;
  ~dramsim_wrapper() override;

private:
  dramsim3::MemorySystem *m_memory_system;
  void receive_read(uint64_t addr);
  void receive_write(uint64_t addr);
  std::queue<uint64_t> read_queue;
  std::queue<uint64_t> write_queue;
  std::queue<uint64_t> read_ret;

};
#endif
#endif // GCN_SIM_DRAMSIM_WRAPPER_H
