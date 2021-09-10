//
// Created by Jiangqiu shen on 5/25/21.
//

#include "controller.h"
#include "debug_helper.h"
#include "spdlog/spdlog.h"
#include <utility>

#include "debug_helper.h"
#include "utils/common.hh"
namespace fast_sched {

// note here, we do not count the latency of read csc format edge from dram to
// on-chip buffer, because the bottleneck should be signal generation process
void controller::handle_buffer_relative_cycle() {
  // 1, send read request to input buffer
  if (i_bf->getNextState() == InputBufferState::empty and
      !task_generation_queue.empty()) {
    // get the next input req and send it to the Inputbuffer
    auto req = std::make_shared<Req>();
    auto &&next_task = task_generation_queue.front();

    // from nodes to address
    std::vector<uint64_t> addrs;
    // from nodes to addres
    for (auto &&i : next_task.input_nodes) {
      uint64_t start_addr = i * currentnodeDim * 4 + currentInputBaseAddr;
      uint64_t end_addr = start_addr + currentnodeDim * 4;
      while (start_addr < end_addr) {
        addrs.push_back(start_addr);
        start_addr += 64;
      }
    }
    req->set_addr(addrs);
    req->t = device_types::input_buffer;
    req->req_type = mem_request::read;
    req->items_cnt = next_task.total_edges;
    // if (next_task.total_edges >= 1000000) {
    //   throw std::runtime_error("total edges too large!!");
    // }
    assert(req->items_cnt != 0);
    req->nodeDim = currentnodeDim;
    assert(currentnodeDim != 0);
    i_bf->send(req);

    task_generation_queue.pop();
  }

  // 2, send task to agg
  // fix bug here,
  // fix again , this not a bug, the edges to be read is obviousely, no need to
  // waiting the control signal, just ignore it!!!
  if (i_bf->getCurrentState() == InputBufferState::readyToRead and
      !agg->isWorking()) {
    // start point, current_sequence_number=1,
    // m_controll_info_generator.get_current_generation_sequence()=0, will need
    // to wait the first control signal.

    current_sequence_number++;
    if (current_sequence_number % 100 == 0) {
      GCN_INFO("finished_controller sequence:{}", current_sequence_number);
    }
    auto &req = i_bf->getCurrentReq();
    assert(req->nodeDim > 0);
    agg->add_task(req, req->nodeDim);
    i_bf->setCurrentState(InputBufferState::reading);
  }

  // 3, check mem return

  if (m_mem->ret_available()) {
    assert(m_mem->peek_req()->t == device_types::input_buffer);

    // spdlog::info("receiving req:{}",
    //              fmt::join(m_mem->peek_req()->get_addr(), ","));
    i_bf->receive(m_mem->get_req());
  }

  // 6, send mem from buffer
  if (m_mem->available()) {
    if (i_bf->getCurrentState() == InputBufferState::readyToSend) {
      // spdlog::info("sending curr_request: addr: {} ",
      //              fmt::join(i_bf->getCurrentReq()->get_addr(), ","));
      m_mem->send(i_bf->getCurrentReq());
      i_bf->setCurrentState(InputBufferState::sent);
    } else if (i_bf->getNextState() == InputBufferState::readyToSend) {

      // spdlog::info("sending next_request: addr: {} ",
      //              fmt::join(i_bf->getNextReq()->get_addr(), ","));
      m_mem->send(i_bf->getNextReq());
      i_bf->setNextState(InputBufferState::sent);
    }
  }

  // 8, change i-buffer stats to be finished when agg finished after reading
  if (!agg->isWorking() and
      i_bf->getCurrentState() == InputBufferState::reading) {
    i_bf->setCurrentState(InputBufferState::empty);
  }
}
void controller::cycle() {
  // update the controller signal

  // need to sync with the controller signal
  // 1, send read request to input buffer
  // 2, send task to agg
  // 3, check mem
  // 4, check working window, fix bug here, remenber to add lines to
  // working_window 5, check pool 6, send memory request to memory 7, check if
  // everything is finished 8, change ibuffer stats

  // first section, handle the buffer relateve functions
  //
  handle_remaining_cycle();

  handle_buffer_relative_cycle();

  handle_task_generation();

  // insert line to work
  handle_work_insert();

  // 7, check finished
  if (pool_all_finished and
      i_bf->getCurrentState() == InputBufferState::empty and
      i_bf->getNextState() == InputBufferState::empty and !agg->isWorking()) {
    all_finished = true;
  }
}

void controller::handle_work_insert() {
  // note that, this function will also remove this flag
  // put a new output node into the buffer ans hashtable
  // fix bug here, when cross the layer, should wait the remaining task to
  // complete
  
  if (this->need_to_insert and remaining_cycle_insert_hash == 0 and
      ((hashtable1.size() * currentnodeDim * 4 + currentnodeDim * 4) <=
       aggBufferSize) and
      large_queue.size() < large_queue_size and
      short_queue.size() < short_queue_size and
      all_tasks_pool.size() < large_queue_size + short_queue_size and
      current_running_layer == currentLayer) {

    // start to insert
    unsigned last_insert = 0;
    while (m_current_pool.have_next_col() or this->next_to_insert_valid) {
      if (m_current_pool.get_next_edge().first != last_insert and
          (hashtable1.size() + 1) * currentnodeDim * 4 > aggBufferSize) {
        fmt::print("fail to insert: current_nodes:{},hashtable1size:{}\n",
                   short_queue.size(), hashtable1.size());
        unable_to_insert_aggbuffer_full++;
        break;
      }
      if (this->next_to_insert_valid) {
        // if (next_to_insert_edge.first == 0) {
        //   GCN_INFO_S("find 0 to insert");
        // }
        throw;
        auto result1 = hashtable1.insert(this->next_to_insert_edge.first,
                                         this->next_to_insert_edge.second);
        if (result1 == 0) {
          unable_to_insert_hashtable_full1++;
          fmt::print("fail to insert: current_nodes:{},hashtable1 size:{}\n",
                     short_queue.size(), hashtable1.size());
          fmt::print("average subqueue size: {}\n",
                     hashtable1.get_total_size());
          break;
        }
        auto result2 = hashTable2.insert(next_to_insert_edge.second,
                                         next_to_insert_edge.second);
        if (result2 == 0) {
          // fail to insert hashtable 2, need to pop hashtable 1
          hashtable1.delete_last(next_to_insert_edge.first);
          unable_to_insert_hashtable_full2++;
          fmt::print("fail to insert: current_nodes:{},hashtable2 size:{}\n",
                     short_queue.size(), hashtable1.size());

          break;
        }
        // Ok, we successfully insert this two value
        remaining_cycle_insert_hash += result1;
        remaining_cycle_insert_hash += result2;
        next_to_insert_valid = false;
        last_insert = next_to_insert_edge.first;

        // handle short queue or  large queue insert
        bool is_short = m_current_pool.get_node_total_len(
                            next_to_insert_edge.first) <= short_large_divider;

        handle_insert_queue(is_short);

      } else {
        // Fix bug here, why the else block be emtpy before...

        auto item = m_current_pool.get_next_edge_and_move();
        // if (item.first == 0) {
        //   GCN_INFO_S("find 0 to insert");
        // }
        next_to_insert_edge = item;
        auto result1 = hashtable1.insert(item.first, item.second);
        // if (item.second == 13738) {
        //   std::cout << "stop here!" << std::endl;
        //   fmt::print("{}:{}", item.first, item.second);
        // }
        if (result1 == 0) {
          next_to_insert_valid = true;
          unable_to_insert_hashtable_full1++;
          // insert fail
          break;
        }
        auto result2 = hashTable2.insert(item.second, item.first);
        if (result2 == 0) {
          hashtable1.delete_last(item.first);
          next_to_insert_valid = true;
          unable_to_insert_hashtable_full2++;

          break;
        }

        remaining_cycle_insert_hash += result1;

        remaining_cycle_insert_hash += result2;
        assert(next_to_insert_valid == false);
        bool is_short = m_current_pool.get_node_total_len(item.first) <=
                        short_large_divider;

        last_insert = item.first;

        // if (item.first == 0) {
        //   GCN_INFO("insert:{}", 0);
        // }
        handle_insert_queue(is_short);
      }
    }

    // update the average window
    while (average_window_size.size() < (current_running_layer + 1)) {
      average_window_size.emplace_back();
    }
    average_window_size[current_running_layer].update(hashtable1.size());
    need_to_insert = false;
  }
  // Old add current task , for reference only
  // while (m_current_pool.have_next_col() and
  //        m_current_work.can_add(m_current_pool.get_next_input_line())) {
  //   auto &&nd = m_current_pool.get_next_input_line_and_move();

  //   static uint64_t print_sign = 0;
  //   if (print_sign % 10 == 0) {
  //     global_definitions.edge_agg_logger->info("edge_buffer: {} {}",
  //                                              m_current_work.get_edge_usage(),
  //                                              (int)config::edgeSize);
  //     global_definitions.edge_agg_logger->info("agg_buffer: {} {}",
  //                                              m_current_work.get_agg_usage(),
  //                                              (int)config::aggSize);
  //   }

  //   print_sign++;

  //   m_current_work.add(nd);
  // }

  // 5, check pool
  if (current_running_layer < currentLayer and hashtable1.empty() and
      i_bf->getCurrentState() == InputBufferState::empty and
      i_bf->getNextState() == InputBufferState::empty and !agg->isWorking()) {
    assert(hashTable2.empty());
    assert(short_queue.empty());
    assert(large_queue.empty());
    assert(all_tasks_pool.empty());
    // fix bug here, because we use seperate insert and task generationlogic, so
    // the meta data change should happend when the real layer changed

    current_running_layer++;
    currentInputBaseAddr += currentnodeDim * 4 * totalNodes;
    currentnodeDim = nodeDims[current_running_layer];
    need_to_insert = true;
    next_to_insert_valid = false;
  }

  if (!m_current_pool.have_next_col() and not pool_all_finished) {
    // waiting until all current layer finished
    if (hashtable1.empty()) {
      assert(hashTable2.empty());
      // move to next layer
      currentLayer++;
      GCN_INFO("switch to next layer:{}", currentLayer);
      if (currentLayer == finalLayer) {
        GCN_INFO("finished the pool:current layer:{}", currentLayer);
        pool_all_finished = true;
      } else {
        GCN_INFO("switch to next layer:layer:{}", currentLayer);
        m_current_pool.reset();
        // set up the environments
      }
    }
  }
}
controller::controller(const Graph &m_graph, const shared_ptr<InputBuffer> &iBf,
                       std::vector<unsigned int> nodeDims,
                       std::vector<unsigned int> inputNodesNum,
                       std::shared_ptr<Aggregator_fast> agg,
                       std::shared_ptr<memory_interface> mMem,
                       unsigned int shortLargeDivider,
                       unsigned int shortQueueSize, unsigned int largeQueueSize,
                       unsigned int taskQueueSize, unsigned int aggBufferSize,
                       bool enable_outer_list, std::string outer_name)
    : short_large_divider(shortLargeDivider),
      hashtable1(config::hash_table_size / 8),
      hashTable2(config::hash_table_size / 8), short_queue_size(shortQueueSize),
      large_queue_size(largeQueueSize), task_queue_size(taskQueueSize),
      m_current_pool(m_graph, enable_outer_list, outer_name), i_bf(iBf),
      nodeDims(std::move(nodeDims)), totalNodes(m_graph.get_num_nodes()),
      currentInputBaseAddr(0x00ff00), currentnodeDim(this->nodeDims[0]),
      currentLayer(0), finalLayer(this->nodeDims.size()),
      m_inputNodeNum(std::move(inputNodesNum)), agg(std::move(agg)),
      m_mem(std::move(mMem)), aggBufferSize(aggBufferSize) {

  if (config::enable_ideal_hash) {
    large_queue_size = 100000;
    short_queue_size = 100000;
    task_queue_size = 100000;
  }
}

bool controller::isAllFinished() const { return all_finished; }
void controller::handle_remaining_cycle() {
  if (remaining_cycle_build_task) {
    remaining_cycle_build_task--;
  }
  if (remaining_cycle_insert_hash) {
    remaining_cycle_insert_hash--;
  }
}

// TODO not tested
void controller::handle_task_generation() {
  // add task
  // TODO
  if (current_running_layer == this->finalLayer) {
    assert(!have_any_element(short_queue, large_queue));
    assert(all_tasks_pool.empty());
    return;
  }

  if (remaining_cycle_build_task == 0 and remaining_cycle_insert_hash == 0) {
    auto task = agg_task{.input_nodes = {}, .total_edges = 0, .edges = {}};

    if (task_generation_queue.size() < task_queue_size) {
      // might send the task
      remaining_cycle_build_task++;
      unsigned in_edge;
      unsigned total_generated_input = 0;
      // bool have_next = false;
      // if (config::enable_ideal_selection) {
      //   have_next = !all_tasks_pool.empty();
      // } else {
      //   have_next = have_any_element(short_queue, large_queue);
      // }
      // Fix bug here, should to test emtpy every time!!
      auto max_input_num = m_inputNodeNum.at(current_running_layer);

      // FIX a seriouse BUG here,
      // a huge bug
      // the and operator is prior than ?, so it always be true!!!

      // auto first_element = short_queue[0];
      // auto last_element = first_element + hashtable1.size();
      // if (first_element == 57) {
      //   std::cout << "break here!" << std::endl;
      //   fmt::print("{}\n", fmt::join(hashtable1.get_edges(57), ","));
      // }
      while ((total_generated_input < max_input_num) and
             (config::enable_ideal_selection
                  ? !all_tasks_pool.empty()
                  : have_any_element(short_queue, large_queue))) {

        // select one output node from sorted queue
        unsigned selected_element = -1;

        selected_element =
            config::enable_ideal_selection
                ? all_tasks_pool.begin()->second
                : get_the_first_valid_element(short_queue, large_queue);
        // bool find_zero = false;
        // if (selected_element == 0) {
        //   find_zero = true;
        // }

        // if (selected_element == 0)
        //   GCN_INFO("find zero {}", selected_element);

        // selected one input from the output node, might remove the
        // item

        // fix bug here, we need input edge here, not in_edge

        remaining_cycle_build_task +=
            hashtable1.query_and_delete(selected_element, in_edge);

        if (hashtable1.is_just_removed()) {
          // if (find_zero) {
          //   GCN_INFO("find zero and moved,{}", selected_element);
          // }
          // we need try to insert new nodes
          this->need_to_insert = true;
          if (config::enable_ideal_selection) {
            assert(all_tasks_pool.begin()->second == selected_element);
            assert(!all_tasks_pool.empty());
            all_tasks_pool.erase(all_tasks_pool.begin());
          } else {
            remove_from_queue(short_queue, large_queue, selected_element);
          }
          hashtable1.set_not_just_removed();
        }

        // got the edges from this hashtable.
        // TODO search hashtable 2 to get all influence output nodes
        // note that , this input might not exist in the hashtable 2,
        // because it might be removed by another output but we do not
        // delete it in this output

        // first need to judge if this input exist in hashtable 2
        if (hashTable2.exist(in_edge)) {

          // fix a bug here, only valid node are considered!
          total_generated_input++;

          std::vector<unsigned> all_infected_output;
          auto hash_table_2 =
              hashTable2.query_and_delete(in_edge, all_infected_output);
          // if (hash_table_2 >= 1000000) {
          //   throw std::runtime_error("hashtable 2 too large!!");
          // }

          // delete all entry at hashtable 1 about the infect output:
          // if (in_edge == 12786) {
          //   std::cout << "stop here!" << std::endl;
          // }
          for (auto &&i : all_infected_output) {
            hashtable1.delete_entry(i, in_edge);
          }
          assert(all_infected_output.size() == hash_table_2);
          task.input_nodes.push_back(in_edge);
          task.edges.push_back({in_edge, all_infected_output});

          task.total_edges += hash_table_2;
          remaining_cycle_build_task += hash_table_2;
        } else {
          // for query the hashtable 2
          remaining_cycle_build_task++;
          // GCN_INFO("could not find input id: {},might be touched
          // before",
          //  in_edge);
        }
      } // end while
      if (task.input_nodes.size()) {
        if (current_running_layer == 0) {

          global_definitions.controlloer_layer_0_windows++;
          global_definitions.controller_layer_0_edges += task.total_edges;
          // fmt::print("start print window:\n");
          // fmt::print("current_output_range:{}:{}\n", first_element,
          //            last_element);
          // fmt::print("{}\n", task.input_nodes.size());
          // for (auto &&i : task.edges) {
          //   fmt::print("input:{}\n", i.first);
          //   for (auto &&j : i.second) {
          //     fmt::print("{} ", j);
          //   }
          //   fmt::print("\n");
          // }
          // fmt::print("{}\n", fmt::join(task.input_nodes, ","));
          // fmt::print("\ntotal_edges:{}\n", task.total_edges);
          // fmt::print("\nend print window:\n");
        }
        // assert(task.total_edges != 0);
        static bool the_first = true;

        if (the_first) {
          the_first = false;
          // print the first window!!
          GCN_INFO("generating the first window:edges:{}", task.total_edges);
          GCN_INFO("the edge detail:{}", fmt::join(task.input_nodes, ","));
        }
        task_generation_queue.push(task);
      }
    } // end test the task queue
  }
}
void controller::handle_insert_queue(bool is_short) {
  if (config::enable_ideal_selection) {

    all_tasks_pool.insert(
        {m_current_pool.get_node_total_len(next_to_insert_edge.first),
         next_to_insert_edge.first});

  } else if (config::enable_sequential_selection) {
    // only push to short queue
    if (short_queue.empty() or
        short_queue.back() != next_to_insert_edge.first) {
      short_queue.push_back(next_to_insert_edge.first);
    }
  } else {
    // default case, real simulation
    if (is_short) {
      // should be insert to short
      if (short_queue.empty() or
          short_queue.back() != next_to_insert_edge.first) {
        short_queue.push_back(next_to_insert_edge.first);
        // GCN_INFO("insert to short_buffered!{}:{}",
        //          next_to_insert_edge.first,
        //          next_to_insert_edge.second);
        // if (next_to_insert_edge.first == 0) {
        //   GCN_INFO("insert:{}", 0);
        // }
      }
    } else {
      if (large_queue.empty() or
          large_queue.back() != next_to_insert_edge.first) {
        // if (next_to_insert_edge.first == 0) {
        //   GCN_INFO("insert:{}", 0);
        // }
        // GCN_INFO("insert to large_buffered!{}:{}",
        //          next_to_insert_edge.first,
        //          next_to_insert_edge.second);
        large_queue.push_back(next_to_insert_edge.first);
      }
      // should be insert to large
    }
  }
}
void controller::print() {
  fmt::print("average_window: {}\n", average_window_size[0].get_average());

  fmt::print("unable_to_insert_hashtable_full: {} {}\n",
             unable_to_insert_hashtable_full1,
             unable_to_insert_hashtable_full2);
  fmt::print("unable_to_insert_aggbuffer_full: {}\n",
             unable_to_insert_aggbuffer_full);
}

shortest_node_info_generator::shortest_node_info_generator(unsigned int policy,
                                                           unsigned queue_size)
    : current_policy(policy), queue_size(queue_size) {}

unsigned
shortest_node_info_generator::get_next_shortest(unsigned int &shortest_value) {
  unsigned cycle = 0;
  if (current_policy == 0) {
    // will sort
    auto &entry = current_choose_vector.front();
    shortest_value = entry.node;
    cycle += 1;
  } else if (current_policy == 1) {
    // not sorted
    unsigned shortest = 0;
    for (auto i = 0u; i < current_choose_vector.size(); i++) {
      if (current_choose_vector[i].size <
          current_choose_vector[shortest].size) {

        shortest = i;
      }
    }
    shortest_value = current_choose_vector[shortest].node;
    cycle += queue_size;
  }
  return cycle;
}
unsigned shortest_node_info_generator::finished_output(
    unsigned int, const std::vector<unsigned int> &, bool is_final) {
  if (current_policy == 0) {
    // the queue is ordered, only re-sort when some node will be removed
    if (is_final) {
      // this is the final one, need to resort all

    } else {
      // not the final one?? what to do?
    }
  }

  return 0;
}

} // namespace fast_sched