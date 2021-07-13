#include <ranges>
void foo() {
  using namespace std;
  for (auto i :
       views::iota(0) | views::transform(|| (auto j) { return j * j; })) {
  }
}