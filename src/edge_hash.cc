#include "edge_hash.h"
namespace sjq {

edge_hash::edge_hash(unsigned int t_e) : total_entry(t_e) {}
std::string edge_hash::get_line_trace() {
  std::string ret;
  for (auto i : entrys) {
    ret += fmt::format("{}: len:{},tag:{}\n", i.first, i.second.get_total_len(),
                       i.second.get_tag());
  }
  return ret;
}

unsigned edge_hash::insert(unsigned int node_id, unsigned value) {

  if (config::enable_ideal_hash1) {
    if (entrys.count(node_id)) {
      entrys.at(node_id).add_edge(value);
    } else {
      sjq::entry_edge m_entry;
      m_entry.set_tag(node_id);
      m_entry.add_edge(node_id);
      entrys.insert({node_id, m_entry});
    }
    return 1;
  }

  /// no-ideal case
  unsigned total_cycle = 0;
  total_cycle++;

  auto entry_id_1 = hash_func_1(node_id);
  auto entry_id_2 = hash_func_2(node_id);
  unsigned real_entry_id;
  bool found = false;

  // 1, find and append

  if (entrys.count(entry_id_1) and entrys.at(entry_id_1).get_tag() == node_id) {
    real_entry_id = entry_id_1;
    found = true;
  } else if (entrys.count(entry_id_2) and
             entrys.at(entry_id_2).get_tag() == node_id) {
    real_entry_id = entry_id_2;
    found = true;
  } else {
    found = false;
    // not found
  }

  if (found) {
    // find existing entry, just append it
    auto &entry = entrys.at(real_entry_id);
    entry.add_edge(value);
    auto next = entrys.upper_bound(real_entry_id);
    if (next != entrys.end()) {
      // this mean how many entry availiable to be used by real_entry_id
      auto gap = next->first - real_entry_id;
      // this mean how many space needed after insert the new value
      // 1->1,2->2 3->2 4-3, n+2/2 is good for this calculation.
      auto space_needed = (entry.get_total_len() + 1 + 2) / 2;

      if (gap >= space_needed) {
        // ok, good to go,
        GCN_DEBUG("insert {} at {} succeed!,current len: {}", node_id,
                  real_entry_id, entry.get_total_len() + 1);
        total_cycle++;

      } else {
        // no, the space is too busy, need to move
        GCN_DEBUG_S("no enough space");
        GCN_DEBUG("need to move {} at {},because we want to insert {} to {} "
                  "with length:{}",
                  next->second.get_tag(), next->first, node_id, real_entry_id,
                  entry.get_total_len() + 1);

        unsigned max_hop = 1;
        auto result = move(unsigned(next->first), 0, entry.get_total_len() + 1,
                           1, max_hop);

        global_definitions.number_hops_histogram[max_hop]++;
        if (result == 0) {
          return 0;
        }
        total_cycle += result;
        GCN_DEBUG("insert {} at {} succeed!,current len: {}", node_id,
                  real_entry_id, entry.get_total_len() + 1);
        total_cycle++;
      }
    } else {
      // reach the end,
      // if fit in the space, just insert
      auto space_needed = (entry.get_total_len() + 1 + 2) / 2;
      auto gap = total_entry - real_entry_id;
      if (gap >= space_needed) {
        GCN_DEBUG("insert {} at {} succeed!,current len: {}", node_id,
                  real_entry_id, entry.get_total_len() + 1);
        total_cycle++;
      } else {
        // try to insert to the other side// from zero
        GCN_DEBUG("space not enought, try to insert to another side:{}",
                  node_id);
        // FIXBUG here, the base shift is need to add
        auto remaining_size = space_needed - gap;
        assert(remaining_size > 0);
        auto begin_it = entrys.begin();
        assert(begin_it != entrys.end());
        auto space_from_begin = begin_it->first;

        // Fix another bug here, may contains multiple conflict
        while (space_from_begin < remaining_size) {

          GCN_DEBUG("fail to insert the node {} at {} , current len:{}, "
                    "need to move {} at {} ",
                    node_id, real_entry_id, entry.get_total_len() + 1,
                    begin_it->second.get_tag(), begin_it->first);
          unsigned max_hop_level = 1;
          auto result = move(unsigned(begin_it->first), 0,
                             entry.get_total_len() + 1, 1, max_hop_level);

          global_definitions.number_hops_histogram[max_hop_level]++;

          if (result == 0) {
            return 0;
          }
          total_cycle += result;

          begin_it = entrys.begin();
          space_from_begin = begin_it->first;
        }

        GCN_DEBUG("insert {} at {} succeed!,current len: {}", node_id,
                  real_entry_id, entry.get_total_len() + 1);
        total_cycle++;
      }
    }
  } else {
    // not found, need to new an entry
    // note that, the entry_id may not be the begining node!!!
    GCN_DEBUG("fail to find the: id {} at {} or {}", node_id, entry_id_1,
              entry_id_2);
    if (is_entry_empty(entry_id_1, 1)) {

      // choose, entry 1
      GCN_DEBUG("put it in entry I:{}", entry_id_1);
      auto t_entry = entry_edge();
      t_entry.set_tag(node_id);
      assert(!entrys.count(entry_id_1));
      t_entry.add_edge(value);
      entrys.insert({entry_id_1, t_entry});
      total_cycle++;

    } else if (is_entry_empty(entry_id_2, 1)) {

      GCN_DEBUG("put it in entry II:{}", entry_id_2);
      auto t_entry = entry_edge();
      t_entry.set_tag(node_id);
      t_entry.add_edge(value);

      assert(!entrys.count(entry_id_2));

      entrys.insert({entry_id_2, t_entry});
      total_cycle++;

    } else {

      // both are not empty, find a shortest one to move
      // this return id is Not the real id!!, the real id should be get from
      // the other function!!!

      auto shortest = find_shortest_to_move(entry_id_1, entry_id_2);

      GCN_DEBUG("fail to find a empty entry, find the shortest:{} in {},{}",
                shortest, entry_id_1, entry_id_2);

      auto shortest_real_entry_id = find_real_entry_id(shortest);

      GCN_DEBUG("move it {} to another place", shortest_real_entry_id);
      unsigned max_hop_level = 1;
      auto result = move(shortest_real_entry_id, shortest, 1, 1, max_hop_level);
      global_definitions.number_hops_histogram[max_hop_level]++;
      if (result == 0) {
        return 0;
      }
      total_cycle += result;
      auto t_entry = entry_edge();
      t_entry.set_tag(node_id);
      t_entry.add_edge(value);

      total_cycle++;
      assert(!entrys.count(shortest));
      entrys.insert({shortest, t_entry});
    }
  }
  return total_cycle;
} // insert

} // namespace sjq
