#ifndef TYPES_H
#define TYPES_H
#include "fmt/format.h"
enum class device_types {
  input_buffer,
  aggregator,
  edge_buffer,
  output_buffer
};
enum class mem_request { read, write };

class Req {
public:
  Req() {
    static unsigned global_id = 0;
    id = global_id;
    global_id++;
    items_cnt = 0;
  }
  unsigned nodeSize;
  unsigned id;
  int items_cnt; //#vertices or #edges
  device_types t;
  mem_request req_type;
  bool the_final_request = false;
  bool the_final_request_of_the_layer = false;
  [[nodiscard]] const std::vector<uint64_t> &get_addr() const {
    return addr;
  }
  [[nodiscard]] uint64_t get_single_addr() const {
    return single_addr;
  }

  [[nodiscard]] bool is_single_addr() const { return use_continue_addr; }
  void set_addr(std::vector<uint64_t> taddr) {
    for (auto &&i : taddr)
      i = i & ~63;
    addr = taddr;
  }
  // will adjust addr and len
  // be carefully
  // this function will set single_addr and single_len
  void set_addr(uint64_t t_addr, unsigned len) {
    use_continue_addr = true;

    // the number of bytes rounded
    // like addr=70, will be rounded to 64, so rounded=6, this part will be
    // added to the total_len. because the start+len should be identical to the
    // original one!
    auto rounded = (t_addr % 63);

    t_addr = t_addr & ~63;
    single_addr = t_addr;
    single_len = len;
    single_len += rounded;
  }

  [[nodiscard]] unsigned get_len() const { return use_continue_addr ? single_len : addr.size(); }

  /*void add64(){
    addr+=64;
  }*/

private:
  std::vector<uint64_t> addr;
  uint64_t single_addr;
  bool use_continue_addr = false;
  unsigned single_len = 0;
};

template <> struct fmt::formatter<Req> {
  bool simple = false;

  constexpr auto parse(format_parse_context &ctx) {

    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin();
    while (it != ctx.end() and *it == 's') {
      if (*it == 's') {
        simple = true;
      }
      it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const Req &p, FormatContext &ctx) {
    // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) //
    // c++11 ctx.out() is an output iterator to write to.
    auto out = format_to(
        ctx.out(), "id:{} addr:{} size:{} type:{} ", p.id,
        p.is_single_addr() ? fmt::format("{}", p.get_single_addr())
                           : fmt::format("[{}]", fmt::join(p.get_addr(), ",")),
        p.get_len(),
        p.t == device_types::input_buffer  ? "Input"
        : p.t == device_types::edge_buffer ? "edge"
                                           : "else");

    return out;
  }
};

#endif /* TYPES_H */
