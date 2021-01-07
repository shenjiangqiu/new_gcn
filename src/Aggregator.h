//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_AGGREGATOR_H
#define GCN_SIM_AGGREGATOR_H

#include "buffer.h"

#include "Slide_window.h"

class Aggregator {
public:
    void add_task();

    bool emtpy();

    bool waiting_buffer();

    void cycle();


private:
    //
    std::shared_ptr<Buffer_base> input_buffer;
    std::shared_ptr<Buffer_base> edge_buffer;
    std::shared_ptr<Buffer_base> agg_buffer;

    int calculate_remaining_cycle();

    std::shared_ptr<Slide_window> current_sliding_window;

    bool empty;
    bool working;
    int remaining_cycles;

};


#endif //GCN_SIM_AGGREGATOR_H
