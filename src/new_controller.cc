// //
// // Created by Jiangqiu shen on 6/30/21.
// //
// #include "new_controller.h"

// namespace sjq {
// using namespace fast_sched;


// new_controller::new_controller() {}


// void new_controller::cycle() {

//   // update the controller signal

//   // 1, send read request to input buffer
//   // 2, send task to agg
//   // 3, check mem
//   // 4, check working window, fix bug here, remenber to add lines to
//   // working_window 5, check pool 6, send memory request to memory 7, check if
//   // everything is finished 8, change ibuffer stats

//   // 1, send read request to input buffer
//   if (i_bf->getNextState() == InputBufferState::empty and
//       m_current_work.have_next_input_node()) {
//     // get the next input req and send it to the Inputbuffer
//     auto req = std::make_shared<Req>();
//     auto next_input_nodes = m_current_work.get_next_input_nodes();
//     static uint64_t print_sign = 0;
//     if (print_sign % WINDOW_PRINT_GAP == 0) {
//       print_window(m_current_work, next_input_nodes, print_sign, "self");
//     }
//     print_sign++;

//     // from nodes to address
//     std::vector<uint64_t> addrs;
//     // from nodes to addres
//     for (auto &&i : next_input_nodes) {
//       uint64_t start_addr = i * currentnodeDim * 4 + currentInputBaseAddr;
//       uint64_t end_addr = start_addr + currentnodeDim * 4;
//       while (start_addr < end_addr) {
//         addrs.push_back(start_addr);
//         start_addr += 64;
//       }
//     }
//     req->set_addr(addrs);
//     req->t = device_types::input_buffer;
//     req->req_type = mem_request::read;
//     req->items_cnt = m_current_work.get_current_item_count();
//     req->nodeDim = currentnodeDim;
//     i_bf->send(req);
//   }

//   // 2, send task to agg
//   // fix bug here,
//   // fix again , this not a bug, the edges to be read is obviousely, no need to
//   // waiting the control signal, just ignore it!!!
//   if (i_bf->getCurrentState() == InputBufferState::readyToRead and
//       !agg->isWorking()) {
//     // start point, current_sequence_number=1,
//     // m_controll_info_generator.get_current_generation_sequence()=0, will need
//     // to wait the first control signal.
//     if (current_sequence_number >
//         m_controll_info_generator.get_current_generation_sequence()) {
//       // controller signal not ready, wait!! note that the controller signal
//       // generation is fully async with agg Do nothing
//     } else {
//       current_sequence_number++;
//       if (current_sequence_number % 100 == 0) {
//         GCN_INFO("finished_controller sequence:{}", current_sequence_number);
//       }
//       auto &req = i_bf->getCurrentReq();
//       assert(req->nodeDim > 0);
//       agg->add_task(req, req->nodeDim);
//       i_bf->setCurrentState(InputBufferState::reading);
//     }
//   }
//   // 3, check mem

//   if (m_mem->ret_available()) {
//     assert(m_mem->peek_req()->t == device_types::input_buffer);
//     i_bf->receive(m_mem->get_req());
//   }
//   // insert line to work
//   while (m_current_pool.have_next_col() and
//          m_current_work.can_add(m_current_pool.get_next_input_line())) {
//     auto &&nd = m_current_pool.get_next_input_line_and_move();

//     static uint64_t print_sign = 0;
//     if (print_sign % 10 == 0) {
//       global_definitions.edge_agg_logger->info("edge_buffer: {} {}",
//                                                m_current_work.get_edge_usage(),
//                                                (int)config::edgeSize);
//       global_definitions.edge_agg_logger->info("agg_buffer: {} {}",
//                                                m_current_work.get_agg_usage(),
//                                                (int)config::aggSize);
//     }

//     print_sign++;

//     m_current_work.add(nd);
//   }

//   // 5, check pool
//   if (!m_current_pool.have_next_col() and not pool_all_finished) {
//     // waiting until all current layer finished
//     if (!m_current_work.have_next_input_node()) {
//       // move to next layer
//       currentLayer++;
//       if (currentLayer == finalLayer) {
//         GCN_INFO("finished the pool:current layer:{}", currentLayer);
//         pool_all_finished = true;
//       } else {
//         GCN_INFO("switch to next layer:layer:{}", currentLayer);
//         m_current_pool.reset();
//         // set up the environments
//         currentInputBaseAddr += currentnodeDim * 4 * totalNodes;
//         assert(currentLayer < nodeDims.size());
//         currentnodeDim = nodeDims[currentLayer];
//         GCN_INFO("setup new window: inputNodes:{} node_size:{}",
//                  m_inputNodeNum[currentLayer], nodeDims[currentLayer] * 4);
//         m_current_work =
//             work(m_inputNodeNum[currentLayer], 4 * nodeDims[currentLayer]);
//       }
//     }
//   }

//   // 6, send mem
//   if (m_mem->available()) {
//     if (i_bf->getCurrentState() == InputBufferState::readyToSend) {
//       m_mem->send(i_bf->getCurrentReq());
//       i_bf->setCurrentState(InputBufferState::sent);
//     } else if (i_bf->getNextState() == InputBufferState::readyToSend) {
//       m_mem->send(i_bf->getNextReq());
//       i_bf->setNextState(InputBufferState::sent);
//     }
//   }

//   // 7, check finished
//   if (pool_all_finished and
//       i_bf->getCurrentState() == InputBufferState::empty and
//       i_bf->getNextState() == InputBufferState::empty and !agg->isWorking()) {
//     all_finished = true;
//   }

//   // 8, change ibuffer stats
//   if (!agg->isWorking() and
//       i_bf->getCurrentState() == InputBufferState::reading) {
//     i_bf->setCurrentState(InputBufferState::empty);
//   }
// }
// } // namespace sjq