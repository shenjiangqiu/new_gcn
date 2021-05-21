//
// Created by sjq on 1/6/21.
//

#include "Aggregator.h"

#include "globals.h"
#include "spdlog/spdlog.h"
#include <utility>

#include "debug_helper.h"
#include <algorithm>
Aggregator::Aggregator(const shared_ptr<InputBuffer> &inputBuffer,
                       const shared_ptr<EdgeBuffer> &edgeBuffer,
                       const shared_ptr<Aggregator_buffer> &aggBuffer,
                       int totalCores)
    : input_buffer(inputBuffer), edge_buffer(edgeBuffer), agg_buffer(aggBuffer),
      total_cores(totalCores) {
  cur_layer = 0;
}

void Aggregator::cycle() {

  if (working) {
    // current running one task
    assert(remaining_cycles and current_sliding_window);
    remaining_cycles--;
    if (remaining_cycles == 0) {
      working = false;
      if (config::enable_dense_window)
        GCN_DEBUG(
            "aggregator finished run task. width(x): {} hight(y): {},cycle: {}",
            current_sliding_window->getX(),
            fmt::join(current_sliding_window->getY(), ","),
            global_definitions.cycle);
      else
        GCN_DEBUG(
            "aggregator finished run task. width(x): {} hight(y): {},cycle: {}",
            current_sliding_window->getX(), current_sliding_window->getY_c(),
            global_definitions.cycle);

      input_buffer->finished_current();
      // decide if finished edge buffer
      if (current_sliding_window->isTheFinalRow()) {
        edge_buffer->finished_current();
        agg_buffer->finish_write();
      }
    }
  }
  if (!working) {
    // have task but not running
    assert(remaining_cycles == 0);
    // if this is the new  col, the agg_buffer should be emtpy
    // have task and finished, need waite the agg buffer move it to rad buffer
    if (!agg_buffer->isWriteEmpty() and agg_buffer->isWriteReady()) {
      global_definitions.total_waiting_agg_write++;
      return;
    }
    if (input_buffer->isCurrentReady() and edge_buffer->isCurrentReady() and
        (agg_buffer->isWriteEmpty() or
         (current_sliding_window and agg_buffer->getWriteWindow()->getX() ==
                                         current_sliding_window->getX()))) {

      current_sliding_window = *(input_buffer->getMCurrentIter());
      remaining_cycles = calculate_remaining_cycle();
      global_definitions.do_aggregate += remaining_cycles;

      working = true;
      cur_layer = current_sliding_window->getLevel();

      if (current_sliding_window->isTheFirstRow()) {
        agg_buffer->add_new_task(current_sliding_window);
      }
      if (config::enable_dense_window)
        GCN_DEBUG("aggregator start to run task x: {} y: {} ,cycle: {}",
                  current_sliding_window->getX(),
                  fmt::join(current_sliding_window->getY(), ","),
                  global_definitions.cycle);
      else
        GCN_DEBUG("aggregator start to run task x: {} y: {} ,cycle: {}",
                  current_sliding_window->getX(),
                  current_sliding_window->getY_c(), global_definitions.cycle);
    } else {
      if (!input_buffer->isCurrentReady()) {
        global_definitions.total_waiting_input++;
        global_definitions.layer_wait_input[cur_layer]++;
      }
      if (!edge_buffer->isCurrentReady()) {
        global_definitions.total_waiting_edge++;
      }
    }
  }
}
/*

void Aggregator::add_task(std::shared_ptr<dense_window> window) {
  assert(empty and !working and remaining_cycles == 0);
  assert(!current_sliding_window);
  current_sliding_window = std::move(window);
  empty = false;
  finished = false;
  GCN_DEBUG("aggregator add aggregator task. x: {} y: {},cycle: {}",
                current_sliding_window->getX(), current_sliding_window->getY(),
                global_definitions.cycle);
}
*/

void Aggregator::updateEdgesHist(int flag, int value) {

  if (flag == EDGE) {
    if (edgesInWindowHist[value] <= 0)
      edgesInWindowHist[value] = 1;
    else
      edgesInWindowHist[value] += 1;
  } else {
    if (inputEffHist[value] <= 0)
      inputEffHist[value] = 1;
    else
      inputEffHist[value] += 1;
  }
}

//123
int Aggregator::calculate_remaining_cycle() {

  // Update here, now the ignored features are not count for calculation
  assert(current_sliding_window);
  auto total_nodes = current_sliding_window->getNumEdgesInWindow();

  auto node_size = current_sliding_window
                       ->getCurrentNodeSize(); // num features in one node//not
                                               // the bytes in one nodes

  if (current_sliding_window->getLevel() == 0)
    node_size -= config::ignore_neighbor;
  assert(node_size > 0);

  auto total_elements = total_nodes * node_size;
  global_definitions.total_input_windows++;
  global_definitions.total_aggregate_op += total_elements;
  global_definitions.total_edges += total_nodes;

  auto input_vertices_cnt = config::enable_dense_window
                                ? current_sliding_window->getY().size()
                                : current_sliding_window->getYw();
  // int edges_cnt = current_sliding_window->getNumEdgesInWindow();
  // float input_efficiency = (float)(edges_cnt)/(float)input_vertices_cnt;
  // int eff = (int)(input_efficiency*1000);
  //  updateEdgesHist(EDGE, edges_cnt);
  //  updateEdgesHist(EFF, eff);

  int level = current_sliding_window->getLevel();
  global_definitions.layer_input_windows[level]++;
  global_definitions.layer_aggregate_op[level] += total_elements;
  global_definitions.layer_edges[level] += total_nodes;
  global_definitions.layer_input_vertics[level] += input_vertices_cnt;

  GCN_DEBUG("total elements: {} ,total size: {}", total_elements,
            total_elements * 4);
  if (config::enable_dense_window)
    GCN_DEBUG("aggregator culculate window cycles. x: {} y: {}, result: {} "
              ",rounds: {}",
              current_sliding_window->getX(),
              fmt::join(current_sliding_window->getY(), ","),
              (total_elements + total_cores - 1) / total_cores,
              global_definitions.cycle);
  else
    GCN_DEBUG("aggregator culculate window cycles. x: {} y: {}, result: {} "
              ",rounds: {}",
              current_sliding_window->getX(), current_sliding_window->getY_c(),
              (total_elements + total_cores - 1) / total_cores,
              global_definitions.cycle);

  double sparse_rate = 0;

  switch (level) {
  case 0:
    sparse_rate = config::feature_sparse_rate0;
    break;

  case 1:
    sparse_rate = config::feature_sparse_rate1;
    break;

  case 2:
    sparse_rate = config::feature_sparse_rate2;
    break;

  default:
    sparse_rate = 0;
  }

  auto total_elements_sparse =
      (total_elements * (1.0 - sparse_rate)); // Yue sparse rate
  auto rounds = (total_elements_sparse + total_cores - 1) / total_cores;
  // read dram latency;
  auto per_round_memory_fetch_time = (total_cores * 4 + 31) / 32;
  // for each round, read the data, and use 1 cycle to process.
  auto total_time = rounds * (per_round_memory_fetch_time + 1);

  global_definitions.layer_do_aggregate[level] += total_time;

  return total_time;
}

bool comp(pair<int, int> a, pair<int, int> b) { return a.second > b.second; }

Aggregator::~Aggregator() {

  /*
    std::vector<std::pair<int, uint64_t>> edgeInfo(edgesInWindowHist.begin(),
    edgesInWindowHist.end()); std::sort(edgeInfo.begin(), edgeInfo.end(), comp);


    std::vector<std::pair<int, uint64_t>> effInfo(inputEffHist.begin(),
    inputEffHist.end()); std::sort(effInfo.begin(), effInfo.end(), comp);

     uint64_t sum_eff, sum_edge;
     sum_eff = 0;
     sum_edge = 0;
     int i;
     for(i = 0; i < edgeInfo.size( ); i++)
       sum_edge += edgeInfo[i].second;
     for(i = 0 ; i < effInfo.size( ); i++)
       sum_eff += effInfo[i].second;


     std::cout<<"Edge_hist ";
     for(i = 0; (i < edgeInfo.size( ) && i<= 20); i++){
       std::cout<<"  "<<edgeInfo[i].first<<"
    "<<(float)edgeInfo[i].second/sum_edge;
     }
     std::cout<<"\n";

     std::cout<<"InputEff_hist ";
     for(i = 0; (i < effInfo.size( ) && i <= 20) ; i++){
       std::cout<<"  "<<effInfo[i].first/1000.0<<"
    "<<(float)effInfo[i].second/sum_eff;
     }
     std::cout<<"\n";
     */
}