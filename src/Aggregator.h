//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_AGGREGATOR_H
#define GCN_SIM_AGGREGATOR_H

#include "buffer.h"

#include "Slide_window.h"

class Aggregator {
public:
    void add_task(std::shared_ptr<Slide_window> window);

    [[nodiscard]] bool isEmpty() const {
        return empty;
    }

    [[nodiscard]] bool isWorking() const {
        return working;
    }
    //TODO read buffer have latency
    void cycle();

    Aggregator(const shared_ptr<ReadBuffer> &inputBuffer, const shared_ptr<ReadBuffer> &edgeBuffer,
               const shared_ptr<Aggregator_buffer> &aggBuffer, int totalCores);


private:
    //
    std::shared_ptr<ReadBuffer> input_buffer;
    std::shared_ptr<ReadBuffer> edge_buffer;
    std::shared_ptr<Aggregator_buffer> agg_buffer;

    int calculate_remaining_cycle();

    std::shared_ptr<Slide_window> current_sliding_window;
    int total_cores;
    bool empty{true};
    bool working{false};
    bool finished{false};
    int remaining_cycles{0};

};


#endif //GCN_SIM_AGGREGATOR_H
