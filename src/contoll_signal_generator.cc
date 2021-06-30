#include "contoll_signal_generator.hh"
#include "utils/common.hh"

/// TODO rebuild this function,1, need 2 hashtables, 1 for normal edge ,2 for reverse hash table
/// and should generate the signal to others to contol others' running logic.
void control_info_generator::cycle() {

  // 1. when have work, do work

  if (working)
    if (next_sequence_remaining_cycle > 0) {
      next_sequence_remaining_cycle--;
      if (next_sequence_remaining_cycle == 0) {
        // this task finished
        sequence++;
        if (sequence % 100 == 0) {
          GCN_INFO("finished_controller_info_window:{}", sequence);
        }
        working = false;
      }
      return;
    }
  // not working

  // should add new task because remaining cycle is 0

  // 2, not working, from working_window to generate working set
  if (m_work.have_next_input_node()) {
    // get the next input req and send it to the Inputbuffer
    // auto valid_nodes = m_work.get_num_valid_work();
    auto next_input_nodes = m_work.get_next_input_nodes();
    // auto total_query = valid_nodes * next_input_nodes.size();
    static uint64_t print_sign = 0;
    if (print_sign % WINDOW_PRINT_GAP == 0) {
      print_window(m_work, next_input_nodes, print_sign, "info");
    }
    print_sign++;
    // noticed here, why we use += not =, because might need additional cycle:
    // insert a new output node!!!! read the next stage.

    // update here, real query
    auto cycle = 1u;
    for (auto input : next_input_nodes) {

      cycle += m_hash_table.query(input);
      global_definitions.total_cycle_query_hash_table += cycle;
    }
    global_definitions.total_cycle_query_hash_table += cycle;

    next_sequence_remaining_cycle += cycle;

    working = true;
  }

  // 3, when there are empty slotes in working window, add them in
  while (m_pool.have_next_col() and
         m_work.can_add(m_pool.get_next_input_line())) {

    auto next_input_line = m_pool.get_next_input_line_and_move();
    // auto line_size = next_input_line.getInputNodes().size();
    m_work.add(next_input_line);

    // update here, real insert
    auto cycle = 0u;
    for (auto input : next_input_line.get_not_processed()) {

      auto ret = m_hash_table.insert(input);
      global_definitions.total_cycle_insert_hash_table += ret;
      if (ret == 0) {
        spdlog::error("fail to insert a line? this line should be handled "
                      "by manuelly serch, might try to insert it again?");
        throw std::runtime_error("error");
      }
      cycle += ret;
      global_definitions.total_cycle_insert_hash_table += ret;
    }

    next_sequence_remaining_cycle += cycle;
  }

  // switch to next layer
  if (!m_pool.have_next_col() and not pool_all_finished) {
    // waiting until all current layer finished
    if (!m_work.have_next_input_node()) {
      // move to next layer
      // should insert a new layer
      GCN_INFO("in_controller_signal:in_controll_info_generator,switch to "
               "next_layer:{}",
               currentLayer + 1);

      currentLayer++;

      if (currentLayer == final_layer) {
        GCN_INFO("in_controller_signal:finished the pool:current layer:{}",
                 currentLayer);
        pool_all_finished = true;
      } else {
        GCN_INFO("in_controller_signal:switch to next layer:layer:{}",
                 currentLayer);
        m_pool.reset();
        // set up the environments
        // fix bug here, the nodesize should be 4xdim
        m_work = fast_sched::current_working_window(
            m_inputNodeNum[currentLayer], 4 * m_nodeDims[currentLayer]);
      }
    }
  }
}
