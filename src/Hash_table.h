#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <cassert>
#include <debug_helper.h>
#include <fmt/format.h>
#include <map>
#include <set>
#include <spdlog/spdlog.h>
#include <vector>
const unsigned NOT_EXIST = -1;

namespace sjq {
struct entry {
  bool valid = false;
  bool next_valid = false;
  bool main = false;
  unsigned node_id = 0;
  unsigned tag = 0;
  unsigned shift = 0;
  unsigned total_len = 0;
};

class hash_table {
public:
  explicit hash_table(unsigned t_e) : total_entry(t_e) {}
  // insert a new node
  [[nodiscard]] std::string get_line_trace() {
    std::string ret;
    for (auto i : entrys) {
      ret += fmt::format("{}: len:{},tag:{}\n", i.first, i.second.total_len,
                         i.second.tag);
    }
    return ret;
  }

  [[nodiscard]] unsigned insert(unsigned node_id) {

    unsigned total_cycle = 0;
    total_cycle++;

    auto entry_id_1 = hash_func_1(node_id);
    auto entry_id_2 = hash_func_2(node_id);
    unsigned real_entry_id;
    bool found = false;

    // 1, find and append

    if (entrys.count(entry_id_1) and entrys.at(entry_id_1).tag == node_id) {
      real_entry_id = entry_id_1;
      found = true;
    } else if (entrys.count(entry_id_2) and
               entrys.at(entry_id_2).tag == node_id) {
      real_entry_id = entry_id_2;
      found = true;
    } else {
      found = false;
      // not found
    }

    if (found) {
      // find existing entry, just append it
      auto &entry = entrys.at(real_entry_id);
      auto next = entrys.upper_bound(real_entry_id);
      if (next != entrys.end()) {
        // this mean how many entry availiable to be used by real_entry_id
        auto gap = next->first - real_entry_id;
        // this mean how many space needed after insert the new value
        // 1->1,2->2 3->2 4-3, n+2/2 is good for this calculation.
        auto space_needed = (entry.total_len + 1 + 2) / 2;

        if (gap >= space_needed) {
          // ok, good to go,
          GCN_DEBUG("insert {} at {} succeed!,current len: {}", node_id,
                    real_entry_id, entry.total_len + 1);
          entry.total_len += 1;
          total_cycle++;

        } else {
          // no, the space is too busy, need to move
          GCN_DEBUG_S("no enough space");
          GCN_DEBUG("need to move {} at {},because we want to insert {} to {} "
                    "with length:{}",
                    next->second.tag, next->first, node_id, real_entry_id,
                    entry.total_len + 1);

          unsigned max_hop = 1;
          auto result =
              move(unsigned(next->first), 0, entry.total_len + 1, 1, max_hop);

          global_definitions.number_hops_histogram[max_hop]++;
          if (result == 0) {
            return 0;
          }
          total_cycle += result;
          GCN_DEBUG("insert {} at {} succeed!,current len: {}", node_id,
                    real_entry_id, entry.total_len + 1);
          total_cycle++;
          entry.total_len += 1;
        }
      } else {
        // reach the end,
        // if fit in the space, just insert
        auto space_needed = (entry.total_len + 1 + 2) / 2;
        auto gap = total_entry - real_entry_id;
        if (gap >= space_needed) {
          GCN_DEBUG("insert {} at {} succeed!,current len: {}", node_id,
                    real_entry_id, entry.total_len + 1);
          total_cycle++;
          entry.total_len += 1;
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
                      node_id, real_entry_id, entry.total_len + 1,
                      begin_it->second.tag, begin_it->first);
            unsigned max_hop_level = 1;
            auto result = move(unsigned(begin_it->first), 0,
                               entry.total_len + 1, 1, max_hop_level);

            global_definitions.number_hops_histogram[max_hop_level]++;

            if (result == 0) {
              return 0;
            }
            total_cycle += result;

            begin_it = entrys.begin();
            space_from_begin = begin_it->first;
          }

          GCN_DEBUG("insert {} at {} succeed!,current len: {}", node_id,
                    real_entry_id, entry.total_len + 1);
          total_cycle++;
          entry.total_len += 1;
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
        auto t_entry = entry();
        t_entry.tag = node_id;
        t_entry.total_len = 1;
        assert(!entrys.contains(entry_id_1));

        entrys.insert({entry_id_1, t_entry});
        total_cycle++;

      } else if (is_entry_empty(entry_id_2, 1)) {

        GCN_DEBUG("put it in entry II:{}", entry_id_2);
        auto t_entry = entry();
        t_entry.tag = node_id;
        t_entry.total_len = 1;
        assert(!entrys.contains(entry_id_2));

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
        auto result =
            move(shortest_real_entry_id, shortest, 1, 1, max_hop_level);
        global_definitions.number_hops_histogram[max_hop_level]++;
        if (result == 0) {
          return 0;
        }
        total_cycle += result;
        auto t_entry = entry();
        t_entry.tag = node_id;
        t_entry.total_len = 1;
        total_cycle++;
        assert(!entrys.contains(shortest));
        entrys.insert({shortest, t_entry});
      }
    }
    return total_cycle;
  }
  //
  [[nodiscard]] unsigned query(unsigned node_id) {
    // currently we assume no conflict and return the number of entries
    // dont' worry, it's constant
    // fix bug here, it's node id, not entry id!!!!

    auto entryId = get_entry_id_from_node_id(node_id);
    auto entry_size = entrys.at(entryId).total_len;
    entrys.erase(entryId);
    // T
    return entry_size;
  }

private:
  // fucnction move, move the entry_id to it's another hash function, note that
  // the entry_id MUST be the real id of the begining node, not from some inner
  // node!!

  // note that , the node id must exist in the hash table!!!
  unsigned get_entry_id_from_node_id(unsigned node_id) {
    auto entry_id_1 = hash_func_1(node_id);
    auto entry_id_2 = hash_func_2(node_id);
    // 1, find and append
    unsigned real_entry_id = 0;
    if (entrys.count(entry_id_1) and entrys.at(entry_id_1).tag == node_id) {
      real_entry_id = entry_id_1;
    } else if (entrys.count(entry_id_2) and
               entrys.at(entry_id_2).tag == node_id) {
      real_entry_id = entry_id_2;
    } else {
      spdlog::error("in get entry form node id,fail to find the entry of "
                    "node_id: {} at {} or {}",
                    node_id, entry_id_1, entry_id_2);
      throw std::runtime_error("cannot get entry id from node id");
    }
    return real_entry_id;
  }

  [[nodiscard]] unsigned move(unsigned entry_id, unsigned a, unsigned b,
                              unsigned move_depth, unsigned &max_hop_level) {
    if (move_depth >= max_hop_level) {
      max_hop_level = move_depth;
    }
    // meet the max move depth.
    spdlog::debug("currently not using the a and b, maybe use it later for "
                  "fixing the bug! {}",
                  a + b);
    GCN_DEBUG("debug to move:{},current depth:{}, current len:{}", entry_id,
              move_depth, entrys.at(entry_id).total_len);

    if (move_depth >= 5) {
      GCN_DEBUG_S("fail to move!, depth is 5");
      return 0;
    }
    // 1. find the next entry
    auto total_cycle = 1;
    assert(entrys.count(entry_id));
    auto &this_entry = entrys.at(entry_id);
    auto tag = this_entry.tag;

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
          GCN_ERROR("Error! contain loop when move:{}", new_entry_id);
        } else {
          runtime_loop.insert(new_entry_id);
        }

        loop_count++;
        if (loop_count == 1000) {
          throw std::runtime_error(
              "more than 1000 loops in move happend, might "
              "have infinit loop here");
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

  std::map<unsigned, entry> entrys;

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

  [[nodiscard]] static unsigned get_entry_size(const entry &e) {
    return (e.total_len + 2) / 2;
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
              prev->second.total_len, inner_id);

    assert(std::prev(neareast)->first +
               get_entry_size(std::prev(neareast)->second) >
           inner_id);
    return prev->first;
  }
  [[nodiscard]] unsigned find_shortest_to_move(unsigned entryId1,
                                               unsigned entryId2) const {
    auto firstId = find_real_entry_id(entryId1);
    auto firstLen = entrys.at(firstId).total_len;
    auto secondId = find_real_entry_id(entryId2);
    auto secondLen = entrys.at(secondId).total_len;
    if (firstLen < secondLen) {
      return entryId1;
    } else {
      return entryId2;
    }
  }
};

} // namespace sjq
#endif