//
// Created by Jiangqiu shen on 5/28/21.
//
#include "catch2/catch_test_macros.hpp"
#include "controller.h"
#include <spdlog/spdlog.h>
using namespace fast_sched;
TEST_CASE("controller_test") {
  spdlog::set_level(spdlog::level::debug);
  auto m_graph = Graph("test");
  auto i_bf = std::make_shared<InputBuffer>();
  auto m_agg = std::make_shared<Aggregator_fast>(128);
  auto m_mem = std::make_shared<memory_interface>("HBM-config.cfg", "", 64);
  auto m_controller = controller(m_graph, i_bf, {2, 2, 2}, {2, 2, 2}, m_agg,
                                 m_mem, 0, 0, 0, 0, 0,false,"");
  while (!m_controller.isAllFinished()) {
    m_controller.cycle();
    i_bf->cycle();
    m_agg->cycle();
    m_mem->cycle();
  }
}