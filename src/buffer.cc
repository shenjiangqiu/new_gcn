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
  if (current_empty and !next_empty) {
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
  }
}

shared_ptr<Req> EdgeBuffer::pop_current() {
  assert(!current_empty and !current_sent);
  current_sent = true;
  auto req = std::make_shared<Req>();
  req->addr = m_current_iter->getEdgeAddr();
  req->len = m_current_iter->getEdgeLen();
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
  m_current_iter = m_set->begin();
  next_empty = false;
  next_sent = false;
  next_ready = false;
  m_next_iter = std::next(m_current_iter);
}


void InputBuffer::cycle() {

  if (current_empty and !next_empty) {
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
  }

}


shared_ptr<Req> InputBuffer::pop_current() {

  assert(!current_empty and !current_sent);
  current_sent = true;
  auto req = std::make_shared<Req>();
  req->addr = m_current_iter->getInputAddr();
  req->len = m_current_iter->getInputLen();
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
  req->len = m_next_iter->getInputLen();
  req->req_type = mem_request::read;
  req->t = device_types::input_buffer;
  next_req = req;
  start_cycle_map[req->id] = global_definitions.cycle;

  return req;
}


void ReadBuffer::receive(shared_ptr<Req> req) {

  auto latency = global_definitions.cycle - start_cycle_map.at(req->id);
  start_cycle_map.erase(req->id);
  switch (req->t) {
  case device_types::edge_buffer:
    global_definitions.total_read_edge_latency += latency;
    global_definitions.total_read_edge_times++;
    global_definitions.total_read_edge_len += req->len;
    break;

  case device_types::input_buffer:
    global_definitions.total_read_input_latency += latency;
    global_definitions.total_read_input_times++;
    global_definitions.total_read_input_len += req->len;
    break;

  default:
    throw std::runtime_error("can't be here");
  }
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
