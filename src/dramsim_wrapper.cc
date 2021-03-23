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
  inflight_req_cnt++;
}

bool dramsim_wrapper::available() const {
  return read_queue.size() < 256 and write_queue.size() < 256;
  //return read_queue.size() < 16 and write_queue.size() < 16;
}

void dramsim_wrapper::cycle() {
  
  my_cycles++;
  if( inflight_req_cnt ){ 
       sum_inflight_req += inflight_req_cnt;
       active_cycles++;
  }     

  
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
  
  m_memory_system->ClockTick();
  
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
  my_cycles = 0;
  active_cycles = 0;
  inflight_req_cnt = 0;
  sum_inflight_req =0;
  finished_read_req = 0;
  finished_write_req = 0;

  m_memory_system = dramsim3::GetMemorySystem(
      config_file, output_dir,
      [this](uint64_t addr) { this->receive_read(addr); },
      [this](uint64_t addr) { this->receive_write(addr); });
  spdlog::info("init dramsim");
}

dramsim_wrapper::~dramsim_wrapper() { 
   
   float mlp = (float)sum_inflight_req/active_cycles;
   float activeRate = (float)active_cycles/my_cycles;

   std::cout<<"Interface MLP "<< mlp <<" memoy activeRate "<<activeRate;
   std::cout<<" BW "<<(finished_read_req+finished_write_req)*64.0/active_cycles;
   std::cout<<" readRqt "<<finished_read_req<<"  writeRqt "<<finished_write_req<<"\n";
   
   m_memory_system->PrintStats();//Yue
   delete m_memory_system; }

void dramsim_wrapper::receive_read(uint64_t addr) { 
     read_ret.push(addr); 
     inflight_req_cnt--;
     finished_read_req++;
}
void dramsim_wrapper::receive_write(uint64_t addr) {
  // do nothing
  inflight_req_cnt--;
  finished_write_req++;
}
#endif