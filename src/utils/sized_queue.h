/**
 * this file contains the sized queue
 * author: Jiangqiu Shen
 *
 */
#include <exception>
#include <queue>

// prepared for signal transfer
template <typename ElementType> class sized_queue {
public:
  using queue = std::queue<ElementType>;

  sized_queue(unsigned size) : _size(size) {}
  // return if the queue is full
  bool full() const { return _m_q.size() == _size; }
  // try to insert, if full and return false, not inserted
  bool try_insert(const ElementType &e) {
    if (full()) {
      return false;
    } else {
      _m_q.push(e);
      return true;
    }
  }
  // insert, if full, raise an exception
  void insert(const ElementType &e) {
    if (full()) {
      throw std::out_of_range("full");
    } else {
      _m_q.push(e);
    }
  }
  // pop, if empty, raise
  const ElementType pop() {
    if (!_m_q.empty()) {
      auto &&ret = _m_q.front();
      _m_q.pop();
      return ret;
    } else {
      throw std::out_of_range("empty");
    }
  }
  // get the from element
  const ElementType &front() const {
    if (!_m_q.empty()) {
      const auto &&ret = _m_q.front();
      return ret;
    } else {
      throw std::out_of_range("empty");
    }
  }

  bool empty() const { return _m_q.empty(); }

private:
  // init by constructor
  const unsigned _size;
  // init by default
  queue _m_q{};
};