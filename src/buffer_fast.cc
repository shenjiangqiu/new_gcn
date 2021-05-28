#include <buffer_fast.h>

#include "debug_helper.h"
#include "globals.h"
#include "spdlog/spdlog.h"
#include <utility>
namespace fast_sched {

const shared_ptr<Req> &InputBuffer::getCurrentReq() const {
  return current_req;
}
const shared_ptr<Req> &InputBuffer::getNextReq() const { return next_req; }
InputBufferState InputBuffer::getCurrentState() const { return current_state; }
InputBufferState InputBuffer::getNextState() const { return next_state; }
void InputBuffer::send(shared_ptr<Req> &req) {
  assert(next_state == InputBufferState::empty);
  next_state = InputBufferState::readyToSend;
  next_req = req;
}
void InputBuffer::cycle() {
  if (current_state == InputBufferState::empty) {
    if (next_state != InputBufferState::empty) {
      assert(next_req);

      current_req = std::move(next_req);
      current_state = next_state;
      next_state = InputBufferState::empty;
    }
  }
}
void InputBuffer::receive(const std::shared_ptr<Req> &req) {
  if (current_state == InputBufferState::sent and current_req->id == req->id) {
    current_state = InputBufferState::readyToRead;
    return;
  } else {
    assert(next_state == InputBufferState::sent and next_req->id == req->id);
    next_state = InputBufferState::readyToRead;
  }
}
void InputBuffer::setCurrentState(InputBufferState currentState) {
  current_state = currentState;
}
void InputBuffer::setNextState(InputBufferState nextState) {
  next_state = nextState;
}

} // namespace fast_sched