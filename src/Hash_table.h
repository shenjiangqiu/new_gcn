#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <cassert>
#include <debug_helper.h>
#include <fmt/format.h>
#include <map>
#include <set>
#include <spdlog/spdlog.h>
#include <utils/common.hh>
#include <vector>

namespace sjq {
class entry {

public:
  void add_edge(unsigned edge_id) { edges.push_back(edge_id); }
  void delete_edge() { edges.pop_back(); }

  unsigned get_total_len() const { return edges.size(); }
  unsigned get_tag() const { return tag; }
  bool is_valid() const { return valid; }
  bool is_next_valid() const { return next_valid; }
  bool is_main() const { return main; }
  const std::vector<unsigned> &get_edges() const { return edges; }

  void set_tag(unsigned _tag) { tag = _tag; }
  void set_node_id(unsigned _node_id) { node_id = _node_id; }
  // when a entry is created, it's just a place holder
  bool is_place_holder() { return edges.empty(); }

private:
  bool valid = false;
  bool next_valid = false;
  bool main = false;
  unsigned node_id = 0;
  unsigned tag = 0;
  unsigned shift = 0;
  unsigned total_len = 0;

  std::vector<unsigned> edges;
};

class hash_table {
public:
  explicit hash_table(unsigned t_e) : total_entry(t_e) {}
  bool empty() const { return entrys.empty(); }
  // insert a new node
  [[nodiscard]] std::string get_line_trace() {
    std::string ret;
    for (auto i : entrys) {
      ret += fmt::format("{}: len:{},tag:{}\n", i.first,
                         i.second.get_total_len(), i.second.get_tag());
    }
    return ret;
  }

  [[nodiscard]] unsigned insert(unsigned node_id, unsigned value);
  //
  void delete_last(unsigned node_id);

  [[nodiscard]] unsigned query_and_delete(unsigned node_id,
                                          std::vector<unsigned> &out_vector);

  bool exist(unsigned node_id) {
    auto entry_id_1 = hash_func_1(node_id);
    auto entry_id_2 = hash_func_2(node_id);
    if (config::enable_ideal_hash2) {
      entry_id_1 = node_id;
      entry_id_2 = node_id;
    }
    // 1, find and append
    if (entrys.count(entry_id_1) and
        entrys.at(entry_id_1).get_tag() == node_id) {
      return true;
    } else if (entrys.count(entry_id_2) and
               entrys.at(entry_id_2).get_tag() == node_id) {
      return true;
    } else {
      return false;
    }
  }
  unsigned get_edge_size() const { return total_edge_size; }
  unsigned size() const { return entrys.size(); }

private:
  // fucnction move, move the entry_id to it's another hash function, note that
  // the entry_id MUST be the real id of the begining node, not from some inner
  // node!!

  // note that , the node id must exist in the hash table!!!
  unsigned get_entry_id_from_node_id(unsigned node_id);

  // added in insert , decreased in remove
  unsigned total_edge_size = 0;
  [[nodiscard]] unsigned move(unsigned entry_id, unsigned a, unsigned b,
                              unsigned move_depth, unsigned &max_hop_level);

  std::map<unsigned, entry> entrys;

  unsigned total_entry;

  // note that when a_s=b_e, it's not overlap in this problem
  // 1-2,size=1, 2-3, size=1 is not overlap
  [[nodiscard]] static bool is_over_lap(unsigned a_s, unsigned a_e,
                                        unsigned b_s, unsigned b_e) {
    return a_s < b_e && b_s < a_e;
  }
  static unsigned get_entry_size_by_element(unsigned elements) {
    if (config::enable_reduced_entry_hash2) {
      return (32 + 12 * elements + 63) / 64;
    }

    return (elements + 2) / 2;
  }
  [[nodiscard]] static unsigned get_entry_size(const entry &e) {

    return get_entry_size_by_element(e.get_total_len());
  }
  [[nodiscard]] unsigned hash_func_1(unsigned node_id) const {
    unsigned rid;
    rid = node_id % (total_entry / 2);
    return rid;
  }
  [[nodiscard]] unsigned hash_func_2(unsigned node_id) const {
    // fix bug here, 0 will map to 0 again!
    auto id = (node_id * 0x1234 % total_entry + (node_id >> 4) + 0x11f1) %
                  (total_entry / 2) +
              (total_entry / 2);
    unsigned rid;
    rid = id;
    return rid;
  }

  // return on of the conflict entry for given entry
  [[nodiscard]] unsigned get_conflict_entry(unsigned entry_id, unsigned size) {
    auto the_next_nearst = entrys.lower_bound(entry_id);
    if (the_next_nearst != entrys.end()) {
      auto start = the_next_nearst->first;
      auto end = start + get_entry_size(the_next_nearst->second);
      // fix bug here, need to add the baseic shift
      if (is_over_lap(start, end, entry_id, entry_id + size)) {
        return start;
      }
    }

    auto prev = the_next_nearst == entrys.begin() ? the_next_nearst
                                                  : std::prev(the_next_nearst);
    if (the_next_nearst == entrys.begin()) {
      // fix bug here, there do not exist prev, so need to test the overflow
      // conjest from the end!!
      auto end = entrys.end();
      if (end != entrys.begin()) {
        end = std::prev(end);
        auto start = end->first;
        auto len = get_entry_size(end->second);
        if (start + len > total_entry) {
          // have overflow,like start =999, len=1, not overflow, start=999,
          // len=2, overflow, because 999 is the last entry when total_entry is
          // 1000
          auto overflow = start + len - total_entry;
          if (overflow > entry_id) {
            // when overflow is 1, entry id is 0, not empty!
            // when overflow is 1, entry is 1, empty!
            return start;
          }
        }
      }
    }

    if (prev != entrys.end()) {
      auto start = prev->first;
      auto end = start + get_entry_size(prev->second);
      if (is_over_lap(start, end, entry_id, entry_id + size)) {
        return start;
      }
    }
    return NOT_EXIST;
  }

  // test if there are any over lap of giving range
  [[nodiscard]] bool is_entry_empty(unsigned entry_id, unsigned size) const {

    bool is_empty = true;
    auto the_next_nearst = entrys.lower_bound(entry_id);
    if (config::enable_single_entry_hash2) {
      if (the_next_nearst == entrys.end()) {
        return true;
      } else if (the_next_nearst->first != entry_id) {

        // in this case we simulate the entry size is always one, so it's never
        // overlap when entry id is not the same.
        return true;
      } else {
        return false;
      }
    }
    if (entrys.count(entry_id)) {
      // incase of placeholder entry;
      return false;
    }
    if (the_next_nearst != entrys.end()) {
      auto start = the_next_nearst->first;
      auto end = start + get_entry_size(the_next_nearst->second);
      // fix bug here, need to add the baseic shift
      if (is_over_lap(start, end, entry_id, entry_id + size)) {
        is_empty = false;
      }
    }
    auto prev = the_next_nearst == entrys.begin() ? the_next_nearst
                                                  : std::prev(the_next_nearst);
    if (the_next_nearst == entrys.begin()) {
      // fix bug here, there do not exist prev, so need to test the overflow
      // conjest from the end!!
      auto end = entrys.end();
      if (end != entrys.begin()) {
        end = std::prev(end);
        auto start = end->first;
        auto len = get_entry_size(end->second);
        if (start + len > total_entry) {
          // have overflow,like start =999, len=1, not overflow, start=999,
          // len=2, overflow, because 999 is the last entry when total_entry is
          // 1000
          auto overflow = start + len - total_entry;
          if (overflow > entry_id) {
            // when overflow is 1, entry id is 0, not empty!
            // when overflow is 1, entry is 1, empty!
            is_empty = false;
          }
        }
      }
      return is_empty;
    }
    if (prev != entrys.end()) {
      auto start = prev->first;
      auto end = start + get_entry_size(prev->second);
      if (is_over_lap(start, end, entry_id, entry_id + size)) {
        is_empty = false;
      }
    }
    return is_empty;
  }
  // given an inner id get the real entry id
  [[nodiscard]] unsigned find_real_entry_id(unsigned inner_id) const {

    // fix bug here, need to find the overflow entry!!!
    // fix bug here, should be lower bound!!
    auto neareast = entrys.lower_bound(inner_id);
    if (neareast != entrys.end()) {
      if (neareast->first == inner_id) {
        return inner_id;
      }
    }
    if (neareast == entrys.begin()) {
      // no prev exsist, which mean the end one must overflow and cover this
      // entry

#ifndef NDEBUG
      auto last = std::prev(entrys.end());
      assert(last->first + get_entry_size(last->second) > total_entry);
      auto overflow = last->first + get_entry_size(last->second) - total_entry;
      assert(overflow > inner_id);
#endif
      return std::prev(entrys.end())->first;
    };
    auto prev = std::prev(neareast);
    GCN_DEBUG("find_prev: id:{},len:{}, the real id to find:{}", prev->first,
              prev->second.get_total_len(), inner_id);

    assert(std::prev(neareast)->first +
               get_entry_size(std::prev(neareast)->second) >
           inner_id);
    return prev->first;
  }
  [[nodiscard]] unsigned find_shortest_to_move(unsigned entryId1,
                                               unsigned entryId2) const {
    auto firstId = find_real_entry_id(entryId1);
    auto firstLen = entrys.at(firstId).get_total_len();
    auto secondId = find_real_entry_id(entryId2);
    auto secondLen = entrys.at(secondId).get_total_len();
    if (firstLen < secondLen) {
      return entryId1;
    } else {
      return entryId2;
    }
  }
};

} // namespace sjq
#endif