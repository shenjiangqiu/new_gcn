//
// Created by sjq on 1/7/21.
//

#include "Aggregator.h"
#include "catch.hpp"
#include "Slide_window.h"
#include "memory_interface.h"
#include "spdlog/spdlog.h"

#include "globals.h"

TEST_CASE("aggregator_test") {
    spdlog::set_level(spdlog::level::debug);
    auto input_buffer = std::make_shared<ReadBuffer>("input buffer");
    auto edge_buffer = std::make_shared<ReadBuffer>("edge buffer");
    auto agg_buffer = std::make_shared<Aggregator_buffer>("agg buffer");
    auto mem_interface = std::make_shared<memory_interface>("DDR4-config.cfg", 64);


    Aggregator m_agg(input_buffer, edge_buffer, agg_buffer, 128);
    auto m_graph = std::make_shared<Graph>("test");
    Slide_window_set m_set(m_graph, {4, 2, 3}, {4, 2, 5}, {1, 2, 3, 1}, 4);
    auto begin = m_set.get_windows().begin();

    auto input_req = std::make_shared<Req>();

    input_req->addr = begin->getInputAddr();
    input_req->len = begin->getInputLen();
    input_req->req_type = mem_request::read;
    input_req->t = device_types::input_buffer;
    input_buffer->add_next(input_req);

    auto edge_req = std::make_shared<Req>();
    edge_req->addr = begin->getEdgeAddr();
    edge_req->len = begin->getEdgeLen();
    edge_req->req_type = mem_request::read;
    edge_req->t = device_types::edge_buffer;
    edge_buffer->add_next(edge_req);

    for (auto i = m_set.get_windows().begin(); i != m_set.get_windows().end(); i++) {
        auto j = std::next(i);

        m_agg.add_task(std::make_shared<Slide_window>(*i));
        //load the prefetch data
        input_buffer->finish_current_move_next();

        //agg_buffer->finish_current_move_next();
        if (j != m_set.get_windows().end()) {
            //prefetch the next input buffer
            auto next_input_req = std::make_shared<Req>();
            next_input_req->addr = j->getInputAddr();
            next_input_req->len = j->getInputLen();
            next_input_req->req_type = mem_request::read;
            next_input_req->t = device_types::input_buffer;
            input_buffer->add_next(next_input_req);
        }

        if (i == m_set.get_windows().begin() or (std::prev(i)->getX() != i->getX() or
                                                 std::prev(i)->getLevel() != i->getLevel())) {
            //which meas, currently we are going to execute a new col.
            //finished current col.
            //prefetch the next col
            edge_buffer->finish_current_move_next();

            auto next_edge_req = std::make_shared<Req>();
            next_edge_req->addr = j->getEdgeAddr();
            next_edge_req->len = j->getEdgeLen();
            next_edge_req->req_type = mem_request::read;
            next_edge_req->t = device_types::edge_buffer;
            edge_buffer->add_next(next_edge_req);
            if (i != m_set.get_windows().begin())
                agg_buffer->finish_write();
            if (agg_buffer->isReadEmpty() and agg_buffer->isReadReady()) {
                agg_buffer->finish_read();
            }

        }


        while (!m_agg.isEmpty()) {
            m_agg.cycle();
            global_definitions.cycle++;

            input_buffer->cycle();
            edge_buffer->cycle();
            agg_buffer->cycle();
            mem_interface->cycle();

            if (input_buffer->isCurrentSendReady() and mem_interface->avaliable()) {
                spdlog::debug("send input current req to mem,addr:{},cycle:{} ", input_buffer->getCurrentReq()->addr,
                              global_definitions.cycle);
                mem_interface->send(input_buffer->pop_current_req());
            }
            if (input_buffer->isNextSendReady() and mem_interface->avaliable()) {
                spdlog::debug("send input next req to mem,addr:{},cycle:{} ", input_buffer->getNextReq()->addr,
                              global_definitions.cycle);

                mem_interface->send(input_buffer->pop_next_req());

            }
            if (edge_buffer->isCurrentSendReady() and mem_interface->avaliable()) {
                spdlog::debug("send edge current req to mem,addr:{},cycle:{} ", edge_buffer->getCurrentReq()->addr,
                              global_definitions.cycle);

                mem_interface->send(edge_buffer->pop_current_req());
            }
            if (edge_buffer->isNextSendReady() and mem_interface->avaliable()) {
                spdlog::debug("send edge next req to mem,addr:{},cycle:{} ", edge_buffer->getNextReq()->addr,
                              global_definitions.cycle);

                mem_interface->send(edge_buffer->pop_next_req());
            }

            if (mem_interface->ret_avaliable()) {
                auto ret = mem_interface->get_req();
                if (ret->t == device_types::input_buffer) {
                    spdlog::debug(" input  req received,cycle:{} ", global_definitions.cycle);

                    input_buffer->accept_req(ret);
                } else {
                    assert(ret->t == device_types::edge_buffer);
                    spdlog::debug(" edge  req received,cycle:{} ", global_definitions.cycle);

                    edge_buffer->accept_req(ret);
                }
            }


        }

    }

}