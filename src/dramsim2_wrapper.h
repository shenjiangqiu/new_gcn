//
// Created by Jianhui on 2/24/2021.
//

#ifndef GCN_SIM_DRAMSIM2_WRAPPER_H
#define GCN_SIM_DRAMSIM2_WRAPPER_H
#include "dram_wrapper.h"
#include "DRAMSim.h"
#include "Callback.h"
#include "queue"
#include "vector"

class dramsim2_wrapper : public dram_wrapper {
public:
  dramsim2_wrapper(const std::string& config_file, const std::string& sys_file);
  void send(uint64_t addr, bool is_write) override;
  [[nodiscard]] bool available(uint64_t addr) const override;
  void cycle() override;
  [[nodiscard]] bool return_available() const override;
  uint64_t pop() override;
  uint64_t get() const override;
  ~dramsim2_wrapper() override;


private:
  [[nodiscard]] unsigned get_channel_num() const;
  [[nodiscard]] static unsigned get_channel_id(uint64_t addr) ;
  DRAMSim::MultiChannelMemorySystem* m_memory_system;
  //void receive_read(uint64_t addr);
  //void receive_write(uint64_t addr);
  void receive_read(uint32_t id, uint64_t addr, uint64_t memCycle);
  void receive_write(uint32_t id, uint64_t addr, uint64_t memCycle);
  std::vector<std::queue<uint64_t>> read_queue;
  std::vector<std::queue<uint64_t>> write_queue;
  std::queue<uint64_t> read_ret;
  uint64_t pending_write_req;

};

#endif // GCN_SIM_DRAMSIM_WRAPPER_H
