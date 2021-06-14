#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <cassert>
#include <map>
#include <vector>
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
  hash_table(unsigned t_e) : total_entry(t_e), m_value(t_e) {}
  // insert a new node
  unsigned insert(unsigned node_id) {
    functional_number_of_entries[node_id]++;
    return 2;
    // TODO! the next code are in-doing, currently we just assume no conflic
    // happened
    unsigned total_cycle = 0;
    total_cycle++;

    auto entry_id_1 = hash_func_1(node_id);
    auto entry_id_2 = hash_func_2(node_id);
    unsigned real_entry_id = 0;
    bool found = false;
    // 1, find and append
    if (entrys.count(entry_id_1) and entrys[entry_id_1].tag == node_id) {
      real_entry_id = entry_id_1;
      found = true;
    } else if (entrys.count(entry_id_2) and entrys[entry_id_2].tag == node_id) {
      real_entry_id = entry_id_2;
      found = true;
    } else {
      found = false;
      // not found
    }

    if (found) {
      auto &entry = entrys[real_entry_id];
      auto next = entrys.upper_bound(real_entry_id);
      if (next != entrys.end()) {
        auto gap = next->first - real_entry_id;
        if (gap > entry.total_len) {
          // ok, good to go,
          entry.total_len += 1;
        } else {
          // no, the space is too busy, need to move
          move(next->first, real_entry_id, entry.total_len + 1);
          entry.total_len += 1;
        }
      } else {
        // reach the end,
        // if fit in the space, just insert
        if (entry.total_len + 1 + real_entry_id < total_entry) {
          entry.total_len += 1;
        } else {
          // try to insert to the other side// from zero

          // FIXBUG here, the base shift is need to add
          auto begin_size = total_entry - entry.total_len - 1 - real_entry_id;
          assert(begin_size > 0);
          auto begin_it = entrys.begin();
          assert(begin_it != entrys.end());
          if (begin_it->first - begin_size <= 0) {
            // need to move
            move(begin_it->first, real_entry_id, entry.total_len + 1);
          } else {
            entry.total_len += 1;
          }
        }
      }
    } else {
      // not found, need to new an entry
      if (is_entry_empty(entry_id_1, 1)) {
        // choose, entry 1
        auto t_entry = entry();
        t_entry.tag = node_id;
        t_entry.total_len = 1;
        entrys[entry_id_1] = t_entry;
      } else if (is_entry_empty(entry_id_2, 1)) {
        auto t_entry = entry();
        t_entry.tag = node_id;
        t_entry.total_len = 1;
        entrys[entry_id_2] = t_entry;
      } else {
        // both are not empty, find a shortest one to move
        auto shortest = find_shortest_to_move(entry_id_1, entry_id_2);
        move(shortest, shortest, 1);
        auto t_entry = entry();
        t_entry.tag = node_id;
        t_entry.total_len = 1;
        entrys[shortest] = t_entry;
      }
    }
  }
  //
  unsigned query(unsigned node_id) {
    // currently we assume no conflict and return the number of entries
    // dont' worry, it's constant
    auto entryId = find_real_entry_id(node_id);
    auto entry_size = entrys[entryId].total_len;
    entrys.erase(entryId);
    // T
    return entry_size;

  }

private:
  unsigned move(unsigned entry_id, unsigned on_fail_node_id, unsigned len) {
    // TODO implemnt it!

    return 0;
  }
  std::map<unsigned, unsigned> functional_number_of_entries;

  std::map<unsigned, entry> entrys;

  unsigned total_entry;

  unsigned total_search_time{0};

  unsigned total_insert_move_time{0};

  unsigned total_invalid_move_time{0};

  unsigned hash_func_1(unsigned node_id) {
    // TODO
  }
  unsigned hash_func_2(unsigned node_id) {
    // TODO
  }
  bool is_entry_empty(unsigned entry_id, unsigned size) {
    // TODO
  }
  unsigned find_real_entry_id(unsigned node_id) {
    // TODO
  }
  unsigned find_shortest_to_move(unsigned entryId1, unsigned entryId2) {
    // TODO
  }
  unsigned get_real_entry_id_by_inner_id(unsigned entryId) {
    // TODO
  }

  std::vector<entry> m_value;
};

} // namespace sjq
#endif