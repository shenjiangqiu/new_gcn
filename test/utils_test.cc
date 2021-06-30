#include <catch2/catch.hpp>
#include <utils/sized_queue.h>
TEST_CASE("size_queue", "[utils]") {
  auto queue = sized_queue<int>(10);
  for (auto i = 0; i < 10; i++) {
    auto result = queue.try_insert(i);
    REQUIRE(result);
  }
  REQUIRE(queue.full());
  REQUIRE_FALSE(queue.try_insert(19));
  REQUIRE_THROWS(queue.insert(19));
  for (auto i = 0; i < 10; i++) {
    auto result = queue.pop();
    REQUIRE(result == i);
  }
  REQUIRE(queue.empty());
  REQUIRE_THROWS(queue.pop());
}