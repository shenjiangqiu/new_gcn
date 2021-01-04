//
// Created by sjq on 1/4/21.
//

#include "Slide_window.h"

#include <utility>

Slide_window::Slide_window(int x, int y, int xw, int yw, int level, uint64_t inputAddr, uint64_t edgeAddr,
                           uint64_t outputAddr, int inputLen, int edgeLen, int outputLen) : x(x), y(y), xw(xw), yw(yw),
                                                                                            level(level),
                                                                                            input_addr(inputAddr),
                                                                                            edge_addr(edgeAddr),
                                                                                            output_addr(outputAddr),
                                                                                            input_len(inputLen),
                                                                                            edge_len(edgeLen),
                                                                                            output_len(outputLen) {}

int Slide_window::getX() const {
    return x;
}

int Slide_window::getY() const {
    return y;
}

int Slide_window::getXw() const {
    return xw;
}

int Slide_window::getYw() const {
    return yw;
}

int Slide_window::getLevel() const {
    return level;
}

uint64_t Slide_window::getInputAddr() const {
    return input_addr;
}

uint64_t Slide_window::getEdgeAddr() const {
    return edge_addr;
}

uint64_t Slide_window::getOutputAddr() const {
    return output_addr;
}

int Slide_window::getInputLen() const {
    return input_len;
}

int Slide_window::getEdgeLen() const {
    return edge_len;
}

int Slide_window::getOutputLen() const {
    return output_len;
}

Slide_window_set::Slide_window_set(std::shared_ptr<Graph> mGraph, std::vector<int> xwS,
                                   std::vector<int> ywS, std::vector<int> nodeSizeS, int totalLevel)
        : m_graph(std::move(mGraph)), xw_s(std::move(xwS)), yw_s(std::move(ywS)), node_size_s(std::move(nodeSizeS)), total_level(totalLevel) {}
