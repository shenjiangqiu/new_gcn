#ifndef COMMON_HH
#define COMMON_HH
#include <cinttypes>
#include <map>
#include <vector>
#define WINDOW_PRINT_GAP 100

// init at controller.cc
extern std::map<unsigned, unsigned> window_compare;
template <typename current_working_window>
void print_window(const current_working_window &m_current_work,
                  const std::vector<unsigned> &next_input_nodes,
                  uint64_t print_sign, const char *name) {
  GCN_INFO("the {} th input window in CONTOLLER_{} is : {} "
           ",current_output_size:{},input_size:{}, "
           "agg_usage:{},edge_usage:{},current_output_node_size:{}",
           print_sign, name, fmt::join(next_input_nodes, ","),
           m_current_work.get_output_size(), m_current_work.get_input_size(),
           m_current_work.get_agg_usage(), m_current_work.get_edge_usage(),
           m_current_work.get_current_output_node_size());
#ifndef NDEBUG
  if (window_compare.contains(print_sign)) {
    assert(window_compare.at(print_sign) == next_input_nodes);

    GCN_INFO("find {} , match!!", print_sign);
    window_compare.erase(print_sign);
  } else {
    window_compare.insert({print_sign, next_input_nodes});
    GCN_INFO("insert: {} with {}", print_sign,
             fmt::join(next_input_nodes, ","));
  }

#endif
}

#endif /* COMMON_HH */
