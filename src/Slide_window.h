//
// Created by sjq on 1/4/21.
//

#ifndef GCN_SIM_SLIDE_WINDOW_H
#define GCN_SIM_SLIDE_WINDOW_H

#include <iostream>
#include <algorithm>
#include <graph.h>
#include <memory>
#include "fmt/format.h"

class Slide_window {
public:
    bool operator==(const Slide_window &rhs) const;

    bool operator!=(const Slide_window &rhs) const;

    Slide_window() = default;

    Slide_window(int x, int y, int xw, int yw, int level, uint64_t inputAddr, uint64_t edgeAddr,
                 uint64_t outputAddr, int inputLen, int edgeLen, int outputLen, int numNodesInWindow,
                 int currentNodeSize);

    [[nodiscard]] int getX() const;

    [[nodiscard]] int getY() const;

    [[nodiscard]] int getXw() const;

    [[nodiscard]] int getYw() const;

    [[nodiscard]] int getLevel() const;

    [[nodiscard]] uint64_t getInputAddr() const;

    [[nodiscard]] uint64_t getEdgeAddr() const;

    [[nodiscard]] uint64_t getOutputAddr() const;

    [[nodiscard]] int getInputLen() const;

    [[nodiscard]] int getEdgeLen() const;

    [[nodiscard]] int getOutputLen() const;

    [[nodiscard]] int getCurrentNodeSize() const;

private:
    int x, y, xw, yw, level;
    uint64_t input_addr, edge_addr, output_addr;
    int input_len, edge_len, output_len;
    int num_nodes_in_window;
    int current_node_size;
public:
    [[nodiscard]] int getNumNodesInWindow() const;
};


template<>
struct fmt::formatter<Slide_window> {
    bool simple = false;

    constexpr auto parse(format_parse_context &ctx) {


        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();
        while (it != ctx.end() and *it == 's') {
            if (*it == 's') {
                simple = true;
            }
            it++;
        }
        if (it != ctx.end() && *it != '}') {
            throw format_error("invalid format");
        }
        return it;
    }


    template<typename FormatContext>
    auto format(const Slide_window &p, FormatContext &ctx) {
        // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
        // ctx.out() is an output iterator to write to.
        auto out = simple ? format_to(ctx.out(), "{} {} {} {} {}", p.getX(), p.getY(), p.getXw(), p.getYw(),
                                      p.getLevel()) : format_to(ctx.out(),
                                                                "x {} y {} {} {} l {} i_a {} e_a {} o_a {} i_l {} e_l {} o_l {} n_n {}",
                                                                p.getX(),
                                                                p.getY(), p.getXw(), p.getYw(), p.getLevel(),
                                                                p.getInputAddr(), p.getEdgeAddr(), p.getOutputAddr(),
                                                                p.getInputLen(), p.getEdgeLen(), p.getOutputLen(),
                                                                p.getNumNodesInWindow());
        return out;
    }
};

class slide_window_set_iterator
        : public std::iterator<std::bidirectional_iterator_tag, Slide_window> {
public:
    slide_window_set_iterator(const std::vector<Slide_window>::iterator &firstIter,
                              const std::vector<std::vector<Slide_window>>::iterator &secondIter,
                              const std::vector<std::vector<std::vector<Slide_window>>>::iterator &thirdIter,
                              std::vector<std::vector<std::vector<Slide_window>>>::iterator thirdEnd) : first_iter(
            firstIter), second_iter(secondIter), third_iter(thirdIter), third_end(thirdEnd) {}


    slide_window_set_iterator &operator++() {
        if (third_iter == third_end) {
            return *this;
        }
        while (third_iter != third_end) {
            while (second_iter != third_iter->end()) {
                while (first_iter != second_iter->end()) {
                    first_iter++;
                    if (first_iter != second_iter->end()) {
                        return *this;
                    }

                }
                second_iter++;
                if (second_iter != third_iter->end()) {
                    first_iter = second_iter->begin();
                    if (first_iter != second_iter->end()) {
                        return *this;
                    }
                }
            }
            third_iter++;
            if (third_iter != third_end) {
                second_iter = third_iter->begin();
                if (second_iter != third_iter->end()) {
                    first_iter = second_iter->begin();

                    if (first_iter != second_iter->end()) {
                        return *this;
                    }
                }
            }
        }
        return *this;
    }

    slide_window_set_iterator operator++(int) {
        auto copy = *this;
        if (third_iter == third_end) {
            return *this;
        }
        while (third_iter != third_end) {
            while (second_iter != third_iter->end()) {
                while (first_iter != second_iter->end()) {
                    first_iter++;
                    if (first_iter != second_iter->end()) {
                        return copy;
                    }

                }
                second_iter++;
                if (second_iter != third_iter->end()) {
                    first_iter = second_iter->begin();
                    if (first_iter != second_iter->end()) {
                        return copy;
                    }
                }
            }
            third_iter++;
            if (third_iter != third_end) {
                second_iter = third_iter->begin();
                if (second_iter != third_iter->end()) {
                    first_iter = second_iter->begin();

                    if (first_iter != second_iter->end()) {
                        return copy;
                    }
                }
            }
        }
        return copy;
    }

    bool have_next_col() {
        auto next_col = std::next(second_iter);
        if (next_col == third_iter->end()) {
            auto next_level = std::next(third_iter);
            if (next_level == third_end or next_level->begin() == next_level->end()) {
                return false;
            }
            return true;

        } else {
            return true;
        }
    }

    slide_window_set_iterator get_next_col() {
        auto next_col = std::next(second_iter);
        if (next_col == third_iter->end()) {
            auto next_level = std::next(third_iter);
            if (next_level == third_end or next_level->begin() == next_level->end()) {
                throw std::runtime_error("no next col,should check before get");
            }
            return slide_window_set_iterator(next_level->begin()->begin(), next_level->begin(), next_level, third_end);

        } else {
            return slide_window_set_iterator(next_col->begin(), next_col, third_iter, third_end);
        }
    }

    Slide_window &operator*() {
        if (third_iter == third_end) {
            throw std::runtime_error("out of bound");
        }
        return *first_iter;
    }

    slide_window_set_iterator &
    setThirdEnd(const std::vector<std::vector<std::vector<Slide_window>>>::iterator &thirdEnd) {
        third_end = thirdEnd;
        return *this;
    }

    //if both is end, then return true
    //if not end, then need to be all equal.
    bool operator==(const slide_window_set_iterator &rhs) const {
        if (third_iter == third_end and rhs.third_iter == rhs.third_end) {
            return true;
        }
        return
                first_iter == rhs.first_iter &&
                second_iter == rhs.second_iter &&
                third_iter == rhs.third_iter &&
                third_end == rhs.third_end;
    }

    bool operator!=(const slide_window_set_iterator &rhs) const {
        return !(rhs == *this);
    }

    slide_window_set_iterator
    setThirdIter(const std::vector<std::vector<std::vector<Slide_window>>>::iterator &thirdIter) {
        third_iter = thirdIter;
        return *this;
    }

private:
    std::vector<Slide_window>::iterator first_iter;
    std::vector<std::vector<Slide_window>>::iterator second_iter;
    std::vector<std::vector<std::vector<Slide_window>>>::iterator third_iter;


    std::vector<std::vector<std::vector<Slide_window>>>::iterator third_end;


};

class Slide_window_set {
public:
    Slide_window_set(std::shared_ptr<Graph> mGraph, std::vector<int> xwS, std::vector<int> ywS,
                     std::vector<int> nodeSizeS, int totalLevel);

    const std::vector<Slide_window> &get_windows() {
        return m_sliding_window_vec;
    }

    slide_window_set_iterator begin() {
        for (auto i = m_sliding_window_multi_level.begin(); i < m_sliding_window_multi_level.end(); i++) {
            for (auto j = i->begin(); j < i->end(); i++) {
                for (auto k = j->begin(); k < j->end(); j++) {
                    return slide_window_set_iterator(k, j, i, m_sliding_window_multi_level.end());
                }
            }
        }
        throw std::runtime_error("cannot find a valid entry for begin");
    }


    slide_window_set_iterator end() {
        return begin().setThirdIter(m_sliding_window_multi_level.end());
    }

private:
    std::shared_ptr<Graph> m_graph;
    std::vector<int> xw_s;
    std::vector<int> yw_s;

    std::vector<int> node_size_s;

    int total_level;

    std::vector<Slide_window>
            m_sliding_window_vec;
    //3 dimension vector: level-col-row-element
    //m_sliding_window_multi_level[1][2][3],layer,1,col,2,row3
    std::vector<std::vector<std::vector<Slide_window>>> m_sliding_window_multi_level;

    std::vector<uint64_t> node_addrs;//for each level;

};


#endif //GCN_SIM_SLIDE_WINDOW_H
