// //
// // Created by Jiangqiu shen on 5/24/21.
// //
// #include "catch2/catch.hpp"
// #include "fast_sched.h"
// #include <globals.h>
// TEST_CASE("fast_sched_test") {
//   config::aggSize = 16777216;
//   config::edgeSize = 2097152;
//   Graph m_graph("test");
//   fast_sched::output_pool m_pool(m_graph);
//   fmt::print("{}\n", m_pool.get_line_trace());

//   fast_sched::current_working_window m_working_window(2, 2);
//   while (m_working_window.can_add(m_pool.get_next_input_line())) {
//     m_working_window.add(m_pool.get_next_input_line_and_move());
//   }

//   fmt::print("{}\n", m_working_window.get_line_trace());
//   fmt::print("{}\n", m_pool.get_line_trace());

//   auto next = m_working_window.get_next_input_nodes();
//   fmt::print("{}\n", m_working_window.get_line_trace());

//   next = m_working_window.get_next_input_nodes();
//   fmt::print("{}\n", m_working_window.get_line_trace());
//   // add a new line
//   while (m_working_window.can_add(m_pool.get_next_input_line())) {
//     m_working_window.add(m_pool.get_next_input_line_and_move());
//   }

//   fmt::print("{}\n", m_working_window.get_line_trace());
//   fmt::print("{}\n", m_pool.get_line_trace());

//   // test all finished

//   m_working_window.get_next_input_nodes();
//   m_working_window.get_next_input_nodes();

//   fmt::print("{}\n", m_working_window.get_line_trace());
//   m_working_window.get_next_input_nodes();
//   fmt::print("{}\n", m_working_window.get_line_trace());
//   m_working_window.get_next_input_nodes();
//   auto ret = m_working_window.get_next_input_nodes();
//   REQUIRE(ret.empty());
//   fmt::print("{}\n", m_working_window.get_line_trace());
// }