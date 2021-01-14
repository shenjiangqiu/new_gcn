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

struct Req {
  Req() {
    static unsigned global_id = 0;
    id = global_id;
    global_id++;
  }

  unsigned id;
  unsigned long long addr;
  unsigned long len;
  device_types t;
  mem_request req_type;
  bool the_final_request = false;
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
    auto out = format_to(ctx.out(), "{} {} {} {} ", p.id, p.addr, p.len,
                         p.t == device_types::input_buffer  ? "Input"
                         : p.t == device_types::edge_buffer ? "edge"
                                                            : "else");

    return out;
  }
};

#endif /* TYPES_H */
