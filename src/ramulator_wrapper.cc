#include "DDR3.h"
#include "DDR4.h"
#include "GDDR5.h"
#include "HBM.h"
#include "LPDDR3.h"
#include "LPDDR4.h"
#include "Memory.h"
#include "MemoryFactory.h"
#include "RamulatorConfig.h"
#include "Request.h"
#include "SALP.h"
#include "WideIO.h"
#include "WideIO2.h"
#include <MemoryFactory.h>
#include <fmt/format.h>
#include <map>
#include <ramulator_wrapper.h>
using namespace ramulator;

static map<string, function<MemoryBase *(const Config &, int)>> name_to_func = {
    {"DDR3", &MemoryFactory<DDR3>::create},
    {"DDR4", &MemoryFactory<DDR4>::create},
    {"LPDDR3", &MemoryFactory<LPDDR3>::create},
    {"LPDDR4", &MemoryFactory<LPDDR4>::create},
    {"GDDR5", &MemoryFactory<GDDR5>::create},
    {"WideIO", &MemoryFactory<WideIO>::create},
    {"WideIO2", &MemoryFactory<WideIO2>::create},
    {"HBM", &MemoryFactory<HBM>::create},
    {"SALP-1", &MemoryFactory<SALP>::create},
    {"SALP-2", &MemoryFactory<SALP>::create},
    {"SALP-MASA", &MemoryFactory<SALP>::create},
};

ramulator_wrapper::ramulator_wrapper(const ramulator::Config configs,
                                     int cacheLine) {
  const string &std_name = configs["standard"];
  assert(name_to_func.find(std_name) != name_to_func.end() &&
         "unrecognized standard name");
  mem = name_to_func[std_name](configs, cacheLine);
  Stats::statlist.output("mem_stats.txt");
  tCK = mem->clk_ns();

  inflight_req_cnt = 0;
  sum_inflight_req = 0;
  my_cycles = 0;
  active_cycles = 0;
  finished_read_req = 0;
  finished_write_req = 0;
  
}
ramulator_wrapper::~ramulator_wrapper() {
  
  float mlp = (float)sum_inflight_req/active_cycles;
  float activeRate = (float)active_cycles/my_cycles;

  std::cout<<"Interface MLP "<< mlp <<" memoy activeRate "<<activeRate;
  std::cout<<" BW "<<(finished_read_req+finished_write_req)*64.0/active_cycles;
  std::cout<<" readRqt "<<finished_read_req<<"  writeRqt "<<finished_write_req<<"\n";
  
  finish();

  delete mem;
}

void ramulator_wrapper::finish() {
  mem->finish();
  Stats::statlist.printall();
}

void ramulator_wrapper::tick() { 
  mem->tick(); 
}

void ramulator_wrapper::send(uint64_t addr, bool is_write) {
  this->in_queue.push({addr, is_write});
  inflight_req_cnt++;
}

void ramulator_wrapper::call_back(ramulator::Request &req) {
  outgoing_reqs--;
  inflight_req_cnt--;

  assert((long long)outgoing_reqs >= 0);
  switch (req.type) {
  case Request::Type::READ:
    out_queue.push(req.addr);
    finished_read_req++;
    break;

  case Request::Type::WRITE:
    finished_write_req++;
    break;

  default:

    break;
  }
}


void ramulator_wrapper::cycle() {
  
  my_cycles++;
  if( inflight_req_cnt ){
        active_cycles++;
        sum_inflight_req +=inflight_req_cnt;
  }

  if (!in_queue.empty()) {
    auto &req = in_queue.front();
    // first addr, second: is_write
    auto r_req = Request(
        req.first, req.second ? Request::Type::WRITE : Request::Type::READ,
        [this](Request &req) { this->call_back(req); });
    if (mem->send(r_req)) {
      outgoing_reqs++;
      in_queue.pop();
    }
  }
  this->tick();
}

bool ramulator_wrapper::empty() const {
  return in_queue.empty() and out_queue.empty() and outgoing_reqs == 0;
}

std::string ramulator_wrapper::get_internal_size() const {
  return fmt::format("name in out outgoing\n mem {} {} {}", in_queue.size(),
                     out_queue.size(), outgoing_reqs);
}

std::string ramulator_wrapper::get_line_trace() const { return ""; }
