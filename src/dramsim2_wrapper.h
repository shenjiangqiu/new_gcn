//
// Created by Jianhui on 2/24/2021.
//

#ifndef GCN_SIM_DRAMSIM2_WRAPPER_H
#define GCN_SIM_DRAMSIM2_WRAPPER_H
#include "dram_wrapper.h"
#include "DRAMSim.h"
#include "Callback.h"
#include "queue"


class dramsim2_wrapper : public dram_wrapper {
public:
  dramsim2_wrapper(const std::string& config_file, const std::string& sys_file);
  void send(uint64_t addr, bool is_write) override;
  bool available() const override;
  void cycle() override;
  bool return_available() const override;
  uint64_t pop() override;
  uint64_t get() const override;
  ~dramsim2_wrapper() override;


private:
  DRAMSim::MultiChannelMemorySystem* m_memory_system;
  //void receive_read(uint64_t addr);
  //void receive_write(uint64_t addr);
  void receive_read(uint32_t id, uint64_t addr, uint64_t memCycle);
  void receive_write(uint32_t id, uint64_t addr, uint64_t memCycle);
  std::queue<uint64_t> read_queue;
  std::queue<uint64_t> write_queue;
  std::queue<uint64_t> read_ret;

};

#endif // GCN_SIM_DRAMSIM_WRAPPER_H
