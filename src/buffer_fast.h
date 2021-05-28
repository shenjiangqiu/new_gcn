#ifndef BUFFER_fast_H
#define BUFFER_fast_H

#include "fast_sched.h"
#include "memory_interface.h"
#include <cassert>
#include <fmt/format.h>
#include <memory>
#include <queue>
#include <ramulator_wrapper.h>
#include <string>
#include <types.h>
#include <utility>
// controller will send this request to input buffer
namespace fast_sched {

enum class InputBufferState { empty, readyToSend, sent, readyToRead, reading };

class InputBuffer {

public:
  void cycle();
  [[nodiscard]] const shared_ptr<Req> &getCurrentReq() const;
  [[nodiscard]] const shared_ptr<Req> &getNextReq() const;
  [[nodiscard]] InputBufferState getCurrentState() const;
  [[nodiscard]] InputBufferState getNextState() const;
  void send(shared_ptr<Req> &req);
  void receive(const std::shared_ptr<Req> &req);

private:
  std::shared_ptr<Req> current_req;
  std::shared_ptr<Req> next_req;
  InputBufferState current_state = InputBufferState::empty;
  InputBufferState next_state = InputBufferState::empty;

public:
  void setCurrentState(InputBufferState currentState);
  void setNextState(InputBufferState nextState);


  // state
};

} // namespace fast_sched

#endif /* BUFFER_fast_H */
