//
// Created by sjq on 1/6/21.
//

#include "Aggregator.h"

#include <utility>

Aggregator::Aggregator(const shared_ptr<ReadBuffer> &inputBuffer, const shared_ptr<ReadBuffer> &edgeBuffer,
                       const shared_ptr<Aggregator_buffer> &aggBuffer, int totalCores) : input_buffer(inputBuffer),
                                                                                         edge_buffer(edgeBuffer),
                                                                                         agg_buffer(aggBuffer),
                                                                                         total_cores(totalCores) {}

void Aggregator::cycle() {
    if (empty) {
        //no task
        return;
    }
    if (working) {
        //current running one task
        assert(remaining_cycles and !finished);
        remaining_cycles--;
        if (remaining_cycles == 0) {
            working = false;
            empty = true;
            finished = true;
        }
    }
    if (!working and !finished) {
        //have task but not running
        assert(remaining_cycles == 0);
        if (input_buffer->isCurrentReady() and edge_buffer->isCurrentReady() and agg_buffer->isNextEmpty()) {
            remaining_cycles = calculate_remaining_cycle();


            working = true;
        }
    }
}

void Aggregator::add_task(std::shared_ptr<Slide_window> window) {
    assert(empty and !working and remaining_cycles == 0);
    assert(!current_sliding_window);
    current_sliding_window = std::move(window);
    empty = false;
}

int Aggregator::calculate_remaining_cycle() {
    assert(current_sliding_window);
    auto total_nodes = current_sliding_window->getNumNodesInWindow();
    auto node_size = current_sliding_window->getCurrentNodeSize();//num features in one node//not the bytes in one nodes
    auto total_elements = total_nodes * node_size;

    return (total_elements + total_cores - 1) / total_cores;
}

