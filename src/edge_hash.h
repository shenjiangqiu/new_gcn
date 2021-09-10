#ifndef EDGE_HASH_H
#define EDGE_HASH_H
#include <cassert>
#include <debug_helper.h>
#include <fmt/format.h>
#include <map>
#include <set>
#include <spdlog/spdlog.h>
#include <utils/common.hh>
#include <vector>
namespace sjq {

struct entry_edge {
  void add_edge(unsigned edge_id) {
    edges.push_back(edge_id);
  }
  void delete_edge(unsigned in) {
// fmt::print("before:{}\n", fmt::join(edges, " ,"));
#ifndef NDEBUG

#endif

    edges.erase(std::remove(edges.begin(), edges.end(), in), edges.end());
    // fmt::print("after:{}\n", fmt::join(edges, " ,"));
  }
  void delete_edge() { edges.pop_back(); }

  unsigned get_total_len() const { return edges.size(); }
  unsigned get_tag() const { return tag; }
  bool is_valid() const { return valid; }
  bool is_next_valid() const { return next_valid; }
  bool is_main() const { return main; }
  const std::vector<unsigned> &get_edges() const { return edges; }

  void set_tag(unsigned _tag) { tag = _tag; }
  void set_node_id(unsigned _node_id) { node_id = _node_id; }
  unsigned get_real_input_id() {
    assert(get_total_len() != 0);

    // return the last edge
    return edges.back();
  }

private:
  bool valid = false;
  bool next_valid = false;
  bool main = false;
  unsigned node_id = 0;
  unsigned tag = 0;
  unsigned shift = 0;

  // total len means the size when inserted

  // real len means the size during runtime, my reduced when other input touch
  // this entry
  // the real edges
  std::vector<unsigned> edges;
};

class edge_hash {

public:
  void delete_entry(unsigned out, unsigned in) {
    if (entrys.contains(out))
      entrys.at(out).delete_edge(in);
  }

  unsigned get_total_size() const {
#ifndef NDEBUG
    auto total = 0;
    for (const auto &i : entrys) {
      total += i.second.get_total_len();
    }
    return total;
#else
    return 0;
#endif
  }
  unsigned size() const { return entrys.size(); }
  explicit edge_hash(unsigned t_e);
  bool empty() const { return entrys.empty(); }
  // insert a new node
  [[nodiscard]] std::string get_line_trace();

  void delete_last(unsigned int node_id) {
    if (config::enable_ideal_hash) {
      entrys.at(node_id).delete_edge();
      return;
    }

    auto entry_id = get_entry_id_from_node_id(node_id);
    entrys.at(entry_id).delete_edge();
  }
  [[nodiscard]] unsigned insert(unsigned node_id, unsigned valude);
  //
  // unsigned query_and_mark(unsigned node_id) {
  //   auto entryId = get_entry_id_from_node_id(node_id);
  //   auto &real_len = entrys.at(entryId).get_total_len();

  //   if (real_len == 0) {
  //     throw std::runtime_error("shouldn't be 0 of real len!!");
  //   }
  //   real_len--;

  //   return 1;
  // }
  // will remove the empty entry
  void remove_entry(unsigned node_id) {
    if (config::enable_ideal_hash) {
      entrys.erase(node_id);
      return;
    }

    auto entryId = get_entry_id_from_node_id(node_id);

    entrys.erase(entryId);
  }

  bool query_is_empty(unsigned node_id) const {

    auto entryId = config::enable_ideal_hash
                       ? node_id
                       : get_entry_id_from_node_id(node_id);

    auto &entry = entrys.at(entryId);
    if (entry.get_total_len() == 0) {
      return true;
    } else {
      return false;
    }
  }
  const std::vector<unsigned> &get_edges(unsigned node_id) {
    return entrys.at(node_id).get_edges();
  }
  // get one node from the node id, remove if the size is zero.
  [[nodiscard]] unsigned query_and_delete(unsigned node_id, unsigned &in_edge) {
    // currently we assume no conflict and return the number of entries
    // dont' worry, it's constant
    // fix bug here, it's node id, not entry id!!!!
    // if (node_id == 15) {
    //   fmt::print("{}\n", fmt::join(entrys.at(15).get_edges(), ", "));
    // }
    auto entryId = get_entry_id_from_node_id(node_id);

    // here, we are going to delete the total len because it's real moved during
    // query, when it's influenced by other input, we are going to reduce the
    // real_len
    assert(entrys.contains(entryId) && "do not contain entryId");
    auto &entry = entrys.at(entryId);
    assert(entry.get_total_len() && "total len is zero!!");
    in_edge = entry.get_edges().back();
    entry.delete_edge();
    if (entry.get_total_len() == 0) {
      just_removed = true;
      entrys.erase(entryId);
    }
    return 1;
  }
  bool is_just_removed() const {
    if (just_removed) {
      return true;
    } else {
      return false;
    }
  };

  // used when alread remove the element in the sorted queue
  void set_not_just_removed() { just_removed = false; }

private:
  // fucnction move, move the entry_id to it's another hash function, note that
  // the entry_id MUST be the real id of the begining node, not from some inner
  // node!!

  // note that , the node id must exist in the hash table!!!
  unsigned get_entry_id_from_node_id(unsigned node_id) const {
    if (config::enable_ideal_hash) {
      return node_id;
    }

    auto entry_id_1 = hash_func_1(node_id);
    auto entry_id_2 = hash_func_2(node_id);
    // 1, find and append
    unsigned real_entry_id = 0;
    if (entrys.count(entry_id_1) and
        entrys.at(entry_id_1).get_tag() == node_id) {
      real_entry_id = entry_id_1;
    } else if (entrys.count(entry_id_2) and
               entrys.at(entry_id_2).get_tag() == node_id) {
      real_entry_id = entry_id_2;
    } else {
      spdlog::error("in get entry form node id,fail to find the entry of "
                    "node_id: {} at {} or {}",
                    node_id, entry_id_1, entry_id_2);
      throw std::runtime_error("cannot get entry id from node id");
    }
    return real_entry_id;
  }
  bool just_removed = false;
  [[nodiscard]] unsigned move(unsigned entry_id, unsigned a, unsigned b,
                              unsigned move_depth, unsigned &max_hop_level) {

    if (config::enable_ideal_hash) {
      return 1;
    }

    if (move_depth >= max_hop_level) {
      max_hop_level = move_depth;
    }
    // meet the max move depth.
    spdlog::debug("currently not using the a and b, maybe use it later for "
                  "fixing the bug! {}",
                  a + b);
    GCN_DEBUG("debug to move:{},current depth:{}, current len:{}", entry_id,
              move_depth, entrys.at(entry_id).get_total_len());

    if (move_depth >= 5) {
      GCN_DEBUG_S("fail to move!, depth is 5");
      return 0;
    }
    // 1. find the next entry
    auto total_cycle = 1;
    assert(entrys.count(entry_id));
    auto &this_entry = entrys.at(entry_id);
    auto tag = this_entry.get_tag();

    // the place that rehash function point to
    auto new_entry_id = 0u;
    if (entry_id == hash_func_1(tag)) {
      new_entry_id = hash_func_2(tag);
    } else {
      new_entry_id = hash_func_1(tag);
    }

    // 2. handle the new conflic
    if (is_entry_empty(new_entry_id, get_entry_size(this_entry))) {
      // just put it here
      // copy to the new location
      GCN_DEBUG("moving: find empty entry:{}, put it in here:{}, len:{}",
                new_entry_id, tag, get_entry_size(this_entry));
      total_cycle += get_entry_size(this_entry);

      assert(!entrys.contains(new_entry_id));
      entrys.insert({new_entry_id, this_entry});
      // release the old one
      entrys.erase(entry_id);
    } else {
      // have entry here
      // move it to new place
      // 1. get all entris in this region
      // rehash point is also conflict, need to move again

      GCN_DEBUG("moving: could not find empty entry at:{} , try to move again!",
                new_entry_id);
      unsigned loop_count = 0;
      std::set<unsigned> runtime_loop;
      while (!is_entry_empty(new_entry_id, get_entry_size(this_entry))) {
        // Fix bug here, there might cause infinit loog
        if (runtime_loop.contains(new_entry_id)) {
          return 0;
        } else {
          runtime_loop.insert(new_entry_id);
        }

        loop_count++;
        if (loop_count == 10) {
          return 0;
        }

        GCN_DEBUG("moving: {} is not empty", new_entry_id);
        auto conf_entry =
            get_conflict_entry(new_entry_id, get_entry_size(this_entry));
        assert(conf_entry != NOT_EXIST);
        total_cycle += move(conf_entry, 0, 0, move_depth + 1, max_hop_level);
      }
      // have already clear this place, put it on!!
      total_cycle += get_entry_size(this_entry);
      assert(!entrys.contains(new_entry_id));
      entrys.insert({new_entry_id, this_entry});
      entrys.erase(entry_id);
    }
    // return the number of cycles, if error happend, return 0 to indicate
    // nothing happend.
    GCN_DEBUG("finished moving:{} to {}, using cycle:{}", entry_id,
              new_entry_id, total_cycle);
    return total_cycle;
  }

  std::map<unsigned, entry_edge> entrys{};

  unsigned total_entry;

  unsigned total_search_time{0};

  unsigned total_insert_move_time{0};

  unsigned total_invalid_move_time{0};

  // note that when a_s=b_e, it's not overlap in this problem
  // 1-2,size=1, 2-3, size=1 is not overlap
  [[nodiscard]] static bool is_over_lap(unsigned a_s, unsigned a_e,
                                        unsigned b_s, unsigned b_e) {
    return a_s < b_e && b_s < a_e;
  }

  [[nodiscard]] static unsigned get_entry_size(const entry_edge &e) {
    return (e.get_total_len() + 2) / 2;
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