//
// Created by ivy22 on 2/1/2021.
//

#ifndef GCN_SIM_DRAMSIM_WRAPPER_H
#define GCN_SIM_DRAMSIM_WRAPPER_H
#include "dram_wrapper.h"
#include "dramsim3.h"
#include "queue"
#include <stdint-gcc.h>
class dramsim_wrapper : public dram_wrapper {
public:
  dramsim_wrapper(const std::string &config_file,
                  const std::string &output_dir);
  void send(unsigned long long addr, bool is_write) override;
  bool available() const override;
  void cycle() override;
  bool return_available() const override;
  unsigned long long pop() override;
  unsigned long long get() const override;
  ~dramsim_wrapper() override;

private:
  dramsim3::MemorySystem *m_memory_system;
  void receive_read(unsigned long long addr);
  void receive_write(unsigned long long addr);
  std::queue<unsigned long long> read_queue;
  std::queue<unsigned long long> write_queue;
  std::queue<unsigned long long> read_ret;

};

#endif // GCN_SIM_DRAMSIM_WRAPPER_H
