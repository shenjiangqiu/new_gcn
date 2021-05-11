#ifndef BUFFER_H
#define BUFFER_H

#include "sliding_window_dense.h"
#include <cassert>
#include <memory>
#include <queue>
#include <ramulator_wrapper.h>
#include <string>
#include <types.h>
#include <utility>
void r(bool &origin);
class Name_object {
public:
  explicit Name_object(std::string name) : name(std::move(name)) {}
  const std::string &get_name() { return name; }

private:
  std::string name;
};
class Buffer_base : public Name_object {
public:
  virtual ~Buffer_base() = default;
  [[nodiscard]] bool isCurrentEmpty() const { return current_empty; }

  [[nodiscard]] bool isCurrentReady() const { return current_ready; }

  [[nodiscard]] bool isNextEmpty() const { return next_empty; }

  [[nodiscard]] bool isNextReady() const { return next_ready; }

  explicit Buffer_base(std::string name) : Name_object(std::move(name)) {}

  // by default, do nothing
  virtual void cycle(){};

  [[nodiscard]] const shared_ptr<Req> &getCurrentReq() const {
    return current_req;
  }

  [[nodiscard]] const shared_ptr<Req> &getNextReq() const { return next_req; }

  virtual void add_next(std::shared_ptr<Req> req) {
    assert(next_empty and !next_ready);
    next_empty = false;
    next_req = std::move(req);
  }

  virtual void finish_current_move_next() {
    current_req = std::move(next_req);
    current_empty = next_empty;
    current_ready = next_ready;

    next_req = nullptr;
    next_empty = true;
    next_ready = false;
  }

protected:
  virtual void add_current(std::shared_ptr<Req> req) {
    assert(current_empty and !current_ready);
    current_empty = false;
    current_req = std::move(req);
  }

  void setCurrentEmpty(bool currentEmpty) { current_empty = currentEmpty; }

  void setCurrentReady(bool currentReady) { current_ready = currentReady; }

  void setNextEmpty(bool nextEmpty) { next_empty = nextEmpty; }

  void setNextReady(bool nextReady) { next_ready = nextReady; }

private:
  std::shared_ptr<Req> current_req;
  std::shared_ptr<Req> next_req;

  bool current_empty{true};
  bool current_ready{false};
  bool next_empty{true};
  bool next_ready{false};
  std::shared_ptr<dense_window> m_window;
};

class Aggregator_buffer : public Name_object {
public:
  explicit Aggregator_buffer(string name);

  void cycle();

  // means no data exists in the write buffer
  [[nodiscard]] bool isWriteEmpty() const;

  // means the data is complete and ready to be read
  [[nodiscard]] bool isWriteReady() const;

  // means the data is ready to be read
  [[nodiscard]] bool isReadEmpty() const;

  // means the data is finished read, can be erased
  [[nodiscard]] bool isReadReady() const;

  [[nodiscard]] const string &getName() const;

  void add_new_task(std::shared_ptr<dense_window> window);
  void start_read();
  void finish_write();

  void finish_read();
  bool isReadBusy() const;

private:
  // for write port;
  bool write_empty{true};
  bool write_ready{false};
  // nothing to be read
  bool read_empty{true};
  // data valid for read
  bool read_ready{false};
  // currently we are reading, do not erase me!
  bool read_busy{false};

  std::shared_ptr<dense_window> read_window;

public:
  [[nodiscard]] const shared_ptr<dense_window> &getReadWindow() const;

  [[nodiscard]] const shared_ptr<dense_window> &getWriteWindow() const;

private:
  std::shared_ptr<dense_window> write_window;

  //
};
/*
class Mem_buffer : public Buffer_base {
public:
  explicit Mem_buffer(const string &basicString);

  void cycle() override;

  virtual std::shared_ptr<Req> pop_current_req() {
    current_send_ready = false;
    return getCurrentReq();
  }

  virtual std::shared_ptr<Req> pop_next_req() {
    next_send_ready = false;
    return getNextReq();
  }

  void add_current(std::shared_ptr<Req> req) override {
    assert(!current_sent and !current_send_ready);
    Buffer_base::add_current(req);
  }

  void add_next(std::shared_ptr<Req> req) override {
    assert(!next_sent and !next_send_ready);
    Buffer_base::add_next(req);
  }

  void finish_current_move_next() override {
    current_send_ready = next_send_ready;
    current_sent = next_sent;

    next_send_ready = false;
    next_sent = false;
    Buffer_base::finish_current_move_next();
  }

  [[nodiscard]] bool isCurrentSent() const { return current_sent; }

  [[nodiscard]] bool isCurrentSendReady() const { return current_send_ready; }

  [[nodiscard]] bool isNextSendReady() const { return next_send_ready; }

  [[nodiscard]] bool isNextSent() const { return next_sent; }

  void accept_req(const std::shared_ptr<Req> &req) {
    if (getNextReq() and getNextReq()->id == req->id) {
      assert(!isNextReady() and !isNextEmpty());
      setNextReady(true);
    } else {
      assert(getCurrentReq()->id == req->id and !isCurrentEmpty() and
             !isCurrentReady());
      setCurrentReady(true);
    }
  }

private:
  bool current_sent{false};
  bool current_send_ready{false};

  bool next_send_ready{false};
  bool next_sent{false};
};
*/
class WriteBuffer : public Name_object {
public:
  explicit WriteBuffer(std::string name) : Name_object(std::move(name)) {}
  void start_write_to_buffer(std::shared_ptr<Req> req) {
    assert(write_to_buffer_empty and !write_to_buffer_started and
           !write_to_buffer_finished);
    r(write_to_buffer_empty);
    r(write_to_buffer_started);
    write_to_buffer_req = std::move(req);
  }

  // finished write to buffer, so it's ready to be moved into memory buffer
  void finished_write_to_buffer();

  // the memory controller start to write back the buffer into memory. so it'
  // not ready to be replaced by write buffer.
  void start_write_memory();

  // the memory buffer is finished write back. So it's ready to be replaced.
  void finished_write_memory();

  // if the write buffer is ready and memory buffer is empty, move the buffer
  void cycle();

  [[nodiscard]] bool is_write_to_buffer_available() const {
    return write_to_buffer_empty;
  }

  [[nodiscard]] bool is_write_to_memory_empty() const;
  [[nodiscard]] bool isWriteToBufferEmpty() const;
  [[nodiscard]] bool isWriteToBufferStarted() const;
  [[nodiscard]] bool isWriteToBufferFinished() const;
  [[nodiscard]] bool isWriteToMemoryEmpty() const;
  [[nodiscard]] bool isWriteToMemoryStarted() const;

  [[nodiscard]] const shared_ptr<Req> &getWriteToMemReq() const;
  const shared_ptr<Req> &popWriteToMemReq() {
    start_write_memory();
    return getWriteToMemReq();
  }

private:
  std::shared_ptr<Req> write_to_buffer_req;
  std::shared_ptr<Req> write_to_mem_req;

  bool write_to_buffer_empty{true};
  bool write_to_buffer_started{false};
  bool write_to_buffer_finished{false};

  bool write_to_memory_empty{true};
  bool write_to_memory_started{false};
};

// only read the edge , do not read edge index now
class ReadBuffer : public Name_object {
public:
  bool idle() { return current_ready and next_ready; }
  explicit ReadBuffer(const string &basicString,
                      const std::shared_ptr<dense_window_set> &m_set);
  virtual ~ReadBuffer() = default;
  virtual void cycle() = 0;

  [[nodiscard]] bool isCurrentReady() const;
  [[nodiscard]] bool isNextEmpty() const;
  [[nodiscard]] bool isNextReady() const;
  [[nodiscard]] bool isCurrentEmpty() const;
  [[nodiscard]] bool isCurrentSent() const;
  [[nodiscard]] bool isNextSent() const;

  [[nodiscard]] bool current_send_ready() const {
    return !current_empty and !current_sent;
  }
  [[nodiscard]] bool next_send_ready() const {
    return !next_empty and !next_sent;
  }
  virtual std::shared_ptr<Req> pop_current() = 0;
  virtual std::shared_ptr<Req> pop_next() = 0;
  virtual void receive(shared_ptr<Req> req);
  void finished_current() {
    assert(!current_empty and current_sent and current_ready);
    current_empty = true;
    current_sent = false;
    current_ready = false;
  }
  [[nodiscard]] const dense_window_iter &getMCurrentIter() const;

protected:
  #define MAX_REQ   32
  int num_buffer_entry; //default is 2, double buffer.
  std::shared_ptr<dense_window_set> m_set;
  dense_window_iter m_current_iter;
  dense_window_iter m_next_iter;
  dense_window_iter final_iter;
  std::shared_ptr<Req> current_req;
  std::shared_ptr<Req> next_req;

  // if current empty but not send, the system should send it,
  bool current_empty{true};
  // if the requests returns, should set return to true;
  bool current_ready{false};
  bool current_sent{false};
  bool next_empty{true};
  bool next_ready{false};
  bool next_sent{false};
  
  int get_ready_buffer_entry( );

  std::map<unsigned long long, unsigned long long> start_cycle_map;
};


class EdgeBuffer : public ReadBuffer {
public:
  explicit EdgeBuffer(const string &name,
                      const std::shared_ptr<dense_window_set> &m_set);
  void cycle() override;
  shared_ptr<Req> pop_current() override;
  shared_ptr<Req> pop_next() override;

protected:
};


class InputBuffer : public ReadBuffer {
public:
  InputBuffer(const std::string &name,
              const std::shared_ptr<dense_window_set> &m_set);
  void cycle() override;
  shared_ptr<Req> pop_current() override;
  shared_ptr<Req> pop_next() override;
};

#endif /* BUFFER_H */
