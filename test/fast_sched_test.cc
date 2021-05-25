//
// Created by Jiangqiu shen on 5/24/21.
//
#include "catch2/catch.hpp"
#include "fast_sched.h"

TEST_CASE("fast_sched_test") {
  Graph m_graph("test");
  fast_ched::output_pool m_poll(m_graph);
  fmt::print("{}\n", m_poll.get_line_trace());

  fast_ched::current_working_window m_working_window(2, 2);
  auto wd = m_working_window.getAllFinishedCol();
  for (auto i : wd) {
    m_working_window.add(i, m_poll.get_next_input_line());
  }

  fmt::print("{}\n", m_working_window.get_line_trace());
  fmt::print("{}\n", m_poll.get_line_trace());

  auto next=m_working_window.get_next_input_nodes();
  fmt::print("{}\n", m_working_window.get_line_trace());

  next=m_working_window.get_next_input_nodes();
  fmt::print("{}\n", m_working_window.get_line_trace());
  //add a new line
  wd=m_working_window.getAllFinishedCol();
  for(auto i:wd){
    m_working_window.add(i,m_poll.get_next_input_line());

  }

  fmt::print("{}\n", m_working_window.get_line_trace());
  fmt::print("{}\n", m_poll.get_line_trace());

  //test all finished

  m_working_window.get_next_input_nodes();
  m_working_window.get_next_input_nodes();

  fmt::print("{}\n", m_working_window.get_line_trace());
  m_working_window.get_next_input_nodes();
  fmt::print("{}\n", m_working_window.get_line_trace());
  m_working_window.get_next_input_nodes();
  auto ret=m_working_window.get_next_input_nodes();
  REQUIRE(ret.empty());
  fmt::print("{}\n", m_working_window.get_line_trace());







}