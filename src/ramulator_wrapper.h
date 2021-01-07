#ifndef RAMULATOR_WRAPPER_H
#define RAMULATOR_WRAPPER_H

#include "Cache.h"
#include "Memory.h"
#include "RamulatorConfig.h"
#include "Request.h"
#include "Statistics.h"
#include <cctype>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <map>
#include <queue>
#include <set>
#include <tuple>

class ramulator_wrapper {
public:
    void send(uint64_t addr, bool is_read);

    void tick();

    void finish();

    ramulator_wrapper(ramulator::Config configs, int cacheLine);

    ~ramulator_wrapper();

    void call_back(ramulator::Request &req);

    [[nodiscard]] bool empty() const;

    [[nodiscard]] std::string get_internal_size() const;

    [[nodiscard]] std::string get_line_trace() const;

    [[nodiscard]] uint64_t get() const { return out_queue.front(); }

    uint64_t pop() {
        auto ret = out_queue.front();
        out_queue.pop();
        return ret;
    }

    [[nodiscard]] bool return_available() const { return !out_queue.empty(); }


    bool do_cycle();

private:
    double tCK;
    unsigned long long outgoing_reqs = 0;
    std::queue<std::pair<uint64_t, bool>> in_queue;
    std::queue<uint64_t> out_queue;
    ramulator::MemoryBase *mem;
};

#endif