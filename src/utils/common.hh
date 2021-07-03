#ifndef COMMON_HH
#define COMMON_HH
#include <cinttypes>
#include <map>
#include <queue>
#include <vector>
const unsigned NOT_EXIST = -1;

#define WINDOW_PRINT_GAP 100

// init at controller.cc
template <typename queueType,
          typename elementType = typename queueType::value_type>
elementType have_any_element(const queueType &q1, const queueType &q2) {
  return (!q1.empty()) or (!q2.empty());
}

template <typename queueType,
          typename elementType = typename queueType::value_type>
void remove_from_queue(queueType &q1, queueType &q2, elementType e) {
  if (q1.front() == e) {
    q1.pop_front();
  } else {
    if (q2.front() != e) {
      throw std::runtime_error("e not exist in q1,q2");
    }
    q2.pop_front();
  }
}

template <typename queueType,
          typename elementType = typename queueType::value_type>
elementType get_the_first_valid_element(const queueType &q1,
                                        const queueType &q2) {
  if (!q1.empty()) {
    auto e = q1.front();
    return e;
  } else {
    assert(!q2.empty());
    auto e = q2.front();
    return e;
  }
}

#endif /* COMMON_HH */
