#include <buffer.h>

#include "globals.h"
#include "spdlog/spdlog.h"
#include <utility>
template <typename T>
void move_to(std::shared_ptr<T> &from, std::shared_ptr<T> &to) {
  to = std::move(from);
  from = nullptr;
}

Aggregator_buffer::Aggregator_buffer(string name)
    : Name_object(std::move(name)) {}

void Aggregator_buffer::cycle() {
  if (write_ready and read_empty) {
    // ready to move;
    assert(!write_empty and !read_ready);
    write_empty = true;
    read_ready = true;

    write_ready = false;
    read_empty = false;
    read_window = std::move(write_window);

    spdlog::debug("aggbuffer move the write buffer to read buffer");
  }
}

bool Aggregator_buffer::isWriteEmpty() const { return write_empty; }

bool Aggregator_buffer::isWriteReady() const { return write_ready; }

bool Aggregator_buffer::isReadEmpty() const { return read_empty; }

bool Aggregator_buffer::isReadReady() const { return read_ready; }

void Aggregator_buffer::add_new_task(std::shared_ptr<Slide_window> window) {
  assert(write_empty and !write_ready and write_window == nullptr);
  r(write_empty);
  write_window = std::move(window);
  spdlog::debug("aggregator add new task:{}", *write_window);
}

void Aggregator_buffer::finish_write() {
  assert(!write_empty and !write_ready);
  r(write_ready);
}

void Aggregator_buffer::finish_read() {
  assert(!read_empty and read_ready and read_busy);
  read_ready = false;
  read_busy = false;
  read_empty = true;
  spdlog::debug("aggbuffer finished read");
}

const shared_ptr<Slide_window> &Aggregator_buffer::getReadWindow() const {
  return read_window;
}

const shared_ptr<Slide_window> &Aggregator_buffer::getWriteWindow() const {
  return write_window;
}
bool Aggregator_buffer::isReadBusy() const { return read_busy; }
void Aggregator_buffer::start_read() {
  assert(read_ready and !read_busy and !read_empty);
  read_busy = true;
}

bool WriteBuffer::is_write_to_memory_empty() const {
  return write_to_memory_empty;
}
void WriteBuffer::cycle() {
  if (write_to_buffer_finished and write_to_memory_empty) {
    assert(!write_to_memory_started);

    r(write_to_memory_empty);

    assert(write_to_buffer_started and !write_to_buffer_empty);
    r(write_to_buffer_empty);
    r(write_to_buffer_started);
    r(write_to_buffer_finished);

    move_to(write_to_buffer_req, write_to_mem_req);
  }
}
bool WriteBuffer::isWriteToBufferEmpty() const { return write_to_buffer_empty; }
bool WriteBuffer::isWriteToBufferStarted() const {
  return write_to_buffer_started;
}
bool WriteBuffer::isWriteToBufferFinished() const {
  return write_to_buffer_finished;
}
bool WriteBuffer::isWriteToMemoryEmpty() const { return write_to_memory_empty; }
bool WriteBuffer::isWriteToMemoryStarted() const {
  return write_to_memory_started;
}

void WriteBuffer::finished_write_to_buffer() {
  assert(write_to_buffer_req and !write_to_buffer_empty and
         write_to_buffer_started and !write_to_buffer_finished);
  r(write_to_buffer_finished);
}
void WriteBuffer::start_write_memory() {

  assert(!write_to_memory_empty and !write_to_memory_started and
         write_to_mem_req);
  r(write_to_memory_started);
}
void WriteBuffer::finished_write_memory() {
  assert(!write_to_memory_empty and write_to_memory_started and
         write_to_mem_req);
  r(write_to_memory_empty);
  r(write_to_memory_started);
  if (write_to_mem_req->the_final_request) {
    global_definitions.finished = true;
  }
  if (write_to_mem_req->the_final_request_of_the_layer) {
    global_definitions.finished_time_stamp.push_back(global_definitions.cycle);
  }
  write_to_mem_req = nullptr;
}

const shared_ptr<Req> &WriteBuffer::getWriteToMemReq() const {
  return write_to_mem_req;
}
void r(bool &origin) { origin = !origin; }

bool ReadBuffer::isCurrentReady() const { return current_ready; }
bool ReadBuffer::isNextEmpty() const { return next_empty; }
bool ReadBuffer::isNextReady() const { return next_ready; }

EdgeBuffer::EdgeBuffer(const string &name,
                       const shared_ptr<Slide_window_set> &m_set)
    : ReadBuffer(name, m_set) {
  current_empty = false;
  current_sent = false;
  current_ready = false;
  m_current_iter = m_set->begin();

  next_empty = false;
  next_sent = false;
  next_ready = false;
  m_next_iter = m_current_iter.get_next_col();
}

void EdgeBuffer::cycle() {

  if (current_empty) {

    if (!next_empty) {

      // move next to current
      m_current_iter = m_next_iter;
      current_empty = next_empty;
      current_ready = next_ready;
      current_sent = next_sent;

      if (m_next_iter.have_next_col()) {
        m_next_iter = m_next_iter.get_next_col();
        next_empty = false;
        next_sent = false;
        next_ready = false;
      } else {
        next_empty = true;
      }
    } else {
      global_definitions.edgeBuffer_idle_cycles++;
    }
  }
}

shared_ptr<Req> EdgeBuffer::pop_current() {
  assert(!current_empty and !current_sent);
  current_sent = true;
  auto req = std::make_shared<Req>();
  req->addr = m_current_iter->getEdgeAddr();
  req->len = m_current_iter->getEdgeLen();
  req->init_len = req->len;
  req->req_type = mem_request::read;
  req->t = device_types::edge_buffer;
  current_req = req;
  start_cycle_map[req->id] = global_definitions.cycle;
  return req;
}

shared_ptr<Req> EdgeBuffer::pop_next() {
  assert(!next_empty and !next_sent);
  next_sent = true;
  auto req = std::make_shared<Req>();
  req->addr = m_next_iter->getEdgeAddr();
  req->len = m_next_iter->getEdgeLen();
  req->init_len = req->len;
  req->req_type = mem_request::read;
  req->t = device_types::edge_buffer;
  next_req = req;
  start_cycle_map[req->id] = global_definitions.cycle;

  return req;
}

InputBuffer::InputBuffer(const string &name,
                         const shared_ptr<Slide_window_set> &m_set)
    : ReadBuffer(name, m_set) {
  current_empty = false;
  current_sent = false;
  current_ready = false;

  next_empty = false;
  next_sent = false;
  next_ready = false;

  /*if( config::enable_multiple_input_buffer ){
     int i = 0;
     buffer_entry_iter[i] = m_set->begin();
     buffer_entry_empty[i] = false;
     buffer_entry_ready[i] = false;
     buffer_entry_sent[i] = false;
     buffer_entry_valid[i] = true;
     for( i = 1; i < num_buffer_entry; i++){
       buffer_entry_iter[i] = std::next(buffer_entry_iter[i-1]);
       buffer_entry_empty[i] = false;
       buffer_entry_ready[i] = false;
       buffer_entry_sent[i] = false;
       buffer_entry_valid[i] = true;
     }
  }else*/
  {
    m_current_iter = m_set->begin();
    m_next_iter = std::next(m_current_iter);
  }
}

/*void InputBuffer::handle_multiple_input_buffer(){
    int i =0;

    //Check if the current input buffere aggregation is done.
    if( current_buffer_done ){
       // 1. setup the reading the next window
       for( i = 0; i < num_buffer_entry; i++){
          if( !buffer_entry_empty[i]){
             // move next to current
            m_current_iter = buffer_entry_iter[i];
            buffer_entry_empty[curr_entry_idx] = true;
            buffer_entry_sent[curr_entry_idx] = false;
            next_iter = std::next(next_iter);
            if( next_iter != std::end())
               buffer_entry_iter[current_indx]= next_iter;
            else{
               buffer_entry_iter[current_indx] =std::end();
               buffer_entry_valid[current_indx] = false;
            }
            current_buffer_done = false;
            return;
          }
       }

    }

    //issue read command
    for( i = 0 ; i < num_buffer_entry; i++){
      if(buffer_entry_sent[i] == false &&
        buffer_entry_valid[i] == true &&
        buffer_entry_empty[i] == true){
          assert( buffer_entry_iter[i] =! std::end());
          // to do issue the read command Yue
        }
      )
    }

}*/

void InputBuffer::cycle() {

  /*if( config::enable_multiple_input_buffer){
       handle_multiple_input_buffer( );
       return;
  }*/
  if (current_empty) {

    if (!next_empty) {
      // move next to current
      m_current_iter = m_next_iter;
      current_empty = next_empty;
      current_ready = next_ready;
      current_sent = next_sent;
      current_req = next_req;

      if (m_next_iter.have_next_row()) {
        m_next_iter++;
        next_empty = false;
        next_sent = false;
        next_ready = false;
      } else {
        next_empty = true;
      }
    } else {
      global_definitions.inputBuffer_idle_cycles++;
    }
  }
}

shared_ptr<Req> InputBuffer::pop_current() {

  assert(!current_empty and !current_sent);
  current_sent = true;
  auto req = std::make_shared<Req>();
  req->addr = m_current_iter->getInputAddr();
  if (config::enable_valid_node_only)
    req->len = m_current_iter->getValidInputLen();
  else
    req->len = m_current_iter->getInputLen();
  req->init_len = req->len;
  req->items_cnt= m_current_iter->getYw();
  req->req_type = mem_request::read;
  req->t = device_types::input_buffer;
  current_req = req;
  start_cycle_map[req->id] = global_definitions.cycle;

  return req;
}

shared_ptr<Req> InputBuffer::pop_next() {

  assert(!next_empty and !next_sent);
  next_sent = true;
  auto req = std::make_shared<Req>();
  req->addr = m_next_iter->getInputAddr();
  if (config::enable_valid_node_only)
    req->len = m_next_iter->getValidInputLen();
  else
    req->len = m_next_iter->getInputLen();
  req->init_len = req->len;
  req->items_cnt= m_next_iter->getYw();
  req->req_type = mem_request::read;
  req->t = device_types::input_buffer;
  next_req = req;
  start_cycle_map[req->id] = global_definitions.cycle;

  return req;
}

/*shared_ptr<Req> InputBuffer::issue_req() {
  int i = 0;
  for( ; i< num_buffer_entry; i++){
       if( buffer_entry_sent[i] == false)
         break;
  }
  if( i == num_buffer_entry)
    return NULL;

  buffer_entry_sent[i] = true;
  auto req = std::make_shared<Req>();
  req->addr = m_next_iter->getInputAddr();
  req->len = m_next_iter->getInputLen();
  req->req_type = mem_request::read;
  req->t = device_types::input_buffer;
  buffer_entry_req[i] = req;
  start_cycle_map[req->id] = global_definitions.cycle;

  return req;
}*/

void ReadBuffer::receive(shared_ptr<Req> req) {

  auto latency = global_definitions.cycle - start_cycle_map.at(req->id);
  start_cycle_map.erase(req->id);
  switch (req->t) {
  case device_types::edge_buffer:
    global_definitions.total_read_edge_latency += latency;
    global_definitions.total_read_edge_times++;
    global_definitions.total_read_edge_len += req->init_len;
    break;

  case device_types::input_buffer:
    global_definitions.total_read_input_latency += latency;
    global_definitions.total_read_input_times++;
    global_definitions.total_read_input_len += req->init_len;
    global_definitions.total_read_input_vertices_cnt += req->items_cnt;
    break;

  default:
    throw std::runtime_error("can't be here");
  }

  /*if( config::enable_multiple_input_buffers ){
    for(i = 0; i < num_buffer_entry; i++){
      if( req->id == buffer_entry_req[i] ){
         buffer_entry_ready[i] = true;
         return;
      }
    }
    return;
  }*/

  if (!current_empty and current_sent and !current_ready and
      req->id == current_req->id) {

    current_ready = true;
  } else {
    assert(!next_empty and next_sent and !next_ready and
           req->id == next_req->id);
    next_ready = true;
  }
}

ReadBuffer::ReadBuffer(const string &basicString,
                       const shared_ptr<Slide_window_set> &m_set)
    : Name_object(basicString), m_set(m_set) {}
const slide_window_set_iterator &ReadBuffer::getMCurrentIter() const {
  return m_current_iter;
}

bool ReadBuffer::isCurrentEmpty() const { return current_empty; }

bool ReadBuffer::isCurrentSent() const { return current_sent; }

bool ReadBuffer::isNextSent() const { return next_sent; }
