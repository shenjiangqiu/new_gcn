//
// Created by sjq on 1/4/21.
//

#ifndef GCN_SIM_SLIDE_WINDOW_H
#define GCN_SIM_SLIDE_WINDOW_H

#include <iostream>
#include <algorithm>
#include <graph.h>
#include <memory>

class Slide_window {
public:

    Slide_window() = default;

    Slide_window(int x, int y, int xw, int yw, int level, uint64_t inputAddr, uint64_t edgeAddr, uint64_t outputAddr,
                 int inputLen, int edgeLen, int outputLen);

    int getX() const;

    int getY() const;

    int getXw() const;

    int getYw() const;

    int getLevel() const;

    uint64_t getInputAddr() const;

    uint64_t getEdgeAddr() const;

    uint64_t getOutputAddr() const;

    int getInputLen() const;

    int getEdgeLen() const;

    int getOutputLen() const;

private:
    int x, y, xw, yw, level;
    uint64_t input_addr, edge_addr, output_addr;
    int input_len, edge_len, output_len;
};

class Slide_window_set {
public:
    Slide_window_set(std::shared_ptr<Graph> mGraph, std::vector<int> xwS, std::vector<int> ywS,
                     std::vector<int> nodeSizeS, int totalLevel);

private:
    std::shared_ptr<Graph> m_graph;
    std::vector<int> xw_s;
    std::vector<int> yw_s;
    std::vector<int> node_size_s;

    int total_level;

    std::vector<Slide_window>
            m_siding_window_vec;

};


#endif //GCN_SIM_SLIDE_WINDOW_H
