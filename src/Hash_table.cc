#include <Hash_table.h>

namespace sjq {

unsigned hash_table::query_and_delete(unsigned int node_id,
                                      std::vector<unsigned> &out) {
  // currently we assume no conflict and return the number of entries
  // dont' worry, it's constant
  // fix bug here, it's node id, not entry id!!!!

  auto entryId = get_entry_id_from_node_id(node_id);
  auto entry_size = entrys.at(entryId).get_total_len();
  out = entrys.at(entryId).get_edges();
  assert(entry_size == out.size());
  entrys.erase(entryId);
  // T
  return entry_size;
}
void sjq::hash_table::delete_last(unsigned int node_id) {
  auto entry_id = get_entry_id_from_node_id(node_id);
  entrys.at(entry_id).delete_edge();
}
unsigned hash_table::insert(unsigned int node_id, unsigned value) {

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
      auto t_entry = entry();
      t_entry.set_tag(node_id);
      t_entry.add_edge(value);
      assert(!entrys.contains(entry_id_1));

      entrys.insert({entry_id_1, t_entry});
      total_cycle++;

    } else if (is_entry_empty(entry_id_2, 1)) {

      GCN_DEBUG("put it in entry II:{}", entry_id_2);
      auto t_entry = entry();
      t_entry.add_edge(value);

      t_entry.set_tag(node_id);

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
      auto result = move(shortest_real_entry_id, shortest, 1, 1, max_hop_level);
      global_definitions.number_hops_histogram[max_hop_level]++;
      if (result == 0) {
        return 0;
      }
      total_cycle += result;
      auto t_entry = entry();
      t_entry.add_edge(value);

      t_entry.set_tag(node_id);

      total_cycle++;
      assert(!entrys.contains(shortest));
      entrys.insert({shortest, t_entry});
    }
  }
  return total_cycle;
}
unsigned hash_table::get_entry_id_from_node_id(unsigned int node_id) {
  auto entry_id_1 = hash_func_1(node_id);
  auto entry_id_2 = hash_func_2(node_id);
  // 1, find and append
  unsigned real_entry_id = 0;
  if (entrys.count(entry_id_1) and entrys.at(entry_id_1).get_tag() == node_id) {
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
unsigned hash_table::move(unsigned int entry_id, unsigned int a, unsigned int b,
                          unsigned int move_depth,
                          unsigned int &max_hop_level) {
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
  GCN_DEBUG("finished moving:{} to {}, using cycle:{}", entry_id, new_entry_id,
            total_cycle);
  return total_cycle;
}
} // namespace sjq