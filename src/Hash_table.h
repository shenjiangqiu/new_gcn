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
    auto entry_id = node_id % total_entry;
    auto &entry = m_value[entry_id];
    if (entry.valid and entry.tag == node_id) {
      // find it and insert it
      auto insert_entry = entry_id + entry.shift;

      auto &new_entry = m_value[insert_entry];
      if (new_entry.valid) {
        assert(new_entry.main);
        auto move_cycle = move(insert_entry, entry_id);
        total_insert_move_time += move_cycle;
        total_cycle += move_cycle;
      }
      new_entry.main = false;
      new_entry.next_valid = false;
      m_value[insert_entry - 1].next_valid = true;
    } else {
      // do not find the
      // TODO
      // 1. not exist
      // 2. be moved
    }
  }
  //
  unsigned query(unsigned node_id) {
    // currently we assume no conflict and return the number of entries
    return functional_number_of_entries.at(node_id);

    // TODO, might be moved!
  }
  unsigned invalid(unsigned node_id) {
    // currently we assume no conflict, just invalid all entries
    auto result = functional_number_of_entries.at(node_id);
    functional_number_of_entries.erase(node_id);
    return result;
  }

private:
  unsigned move(unsigned entry_id, unsigned on_fail_node_id) {
    // todo implemnt it!
    return 0;
  }
  std::map<unsigned, unsigned> functional_number_of_entries;
  unsigned total_entry;

  unsigned total_search_time{0};

  unsigned total_insert_move_time{0};

  unsigned total_invalid_move_time{0};
  std::vector<entry> m_value;
};

} // namespace sjq
#endif