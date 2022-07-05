// #include "catch2/catch_test_macros.hpp"

// #include <Hash_table.h>
// TEST_CASE("hash_table_test") {

//   using namespace sjq;
//   hash_table m_hashtable(1000);
//   spdlog::set_level(spdlog::level::debug);
//   SECTION("simple") {
//     auto cycle = 0;
//     cycle += m_hashtable.insert(199);
//     cycle += m_hashtable.insert(299);
//     cycle += m_hashtable.insert(199);

//     cycle += m_hashtable.insert(1199);
//     cycle += m_hashtable.insert(414);

//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }
//   SECTION("chain") {
//     GCN_DEBUG_S("will insert 660,641,and 1001 to 1, when insert 1, whill try "
//                 "to insert to 1 and 660, and all failed, then evict 660 to "
//                 "641, and then evict 641");
//     auto cycle = 0;

//     cycle += m_hashtable.insert(660);
//     cycle += m_hashtable.insert(641);
//     cycle += m_hashtable.insert(1001);
//     cycle += m_hashtable.insert(1);

//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }
//   SECTION("insert hit and conflict") {
//     auto cycle = 0;
//     cycle += m_hashtable.insert(1);
//     cycle += m_hashtable.insert(3);
//     cycle += m_hashtable.insert(1); // will succeed,entry 2 start
//     cycle += m_hashtable.insert(1); // will succeed, entry 2 full
//     cycle += m_hashtable.insert(1); // will fail, entry 3 start
//     cycle += m_hashtable.insert(1); // will succeed, entry3 full
//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }

//   SECTION("insert miss and conflict") {
//     // occupy 3 entrys
//     auto cycle = 0;
//     cycle += m_hashtable.insert(1);
//     cycle += m_hashtable.insert(1);
//     cycle += m_hashtable.insert(1);
//     cycle += m_hashtable.insert(1);
//     cycle += m_hashtable.insert(1);

//     cycle += m_hashtable.insert(319);
//     cycle += m_hashtable.insert(319);

//     cycle += m_hashtable.insert(2);
//     // will fial because entry 2 is belong to entry 1,
//     // and entry 320 is belong to 319, will move 319
//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }

//   SECTION("move to evict multiple entries") {
//     auto cycle = 0;
//     cycle += m_hashtable.insert(1);
//     // 2 will occupy 3 entries
//     cycle += m_hashtable.insert(2);
//     cycle += m_hashtable.insert(2);

//     cycle += m_hashtable.insert(2);
//     cycle += m_hashtable.insert(2);
//     cycle += m_hashtable.insert(2);

//     // add some element to 320-322 which 2 will be moved to
//     cycle += m_hashtable.insert(320);
//     cycle += m_hashtable.insert(321);
//     cycle += m_hashtable.insert(322);
//     // 323 will not be moved
//     cycle += m_hashtable.insert(323);
//     // will evict 2, and 2 will evict 3 entries
//     cycle += m_hashtable.insert(1);

//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }

//   SECTION("cross the board") {
//     auto cycle = 0;
//     cycle += m_hashtable.insert(999);
//     // will cross the board
//     cycle += m_hashtable.insert(999);
//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }
//   SECTION("cross the board conflic 1") {
//     // overflow will make 0 to insert to another location
//     auto cycle = 0;
//     cycle += m_hashtable.insert(999);
//     // will cross the board
//     cycle += m_hashtable.insert(999);

//     // will conflict because 999

//     // find BUG here, 1, 0 map to two location that belongs to the same location
//     // 0 fail to detect the conflict of 999
//     cycle += m_hashtable.insert(0);
//     // will succeed
//     cycle += m_hashtable.insert(1);

//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }
//   SECTION("corss the board conflict to evict") {
//     // overflow will make 0 to insert to another location
//     auto cycle = 0;

//     cycle += m_hashtable.insert(0);
//     cycle += m_hashtable.insert(999);
//     // will cross the board
//     // will evict 0
//     cycle += m_hashtable.insert(999);

//     // will conflict because 999

//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }
//   SECTION("evict the overflow") {
//     // overflow will make 0 to insert to another location
//     auto cycle = 0;

//     cycle += m_hashtable.insert(593);
//     cycle += m_hashtable.insert(593);
//     cycle += m_hashtable.insert(593);
//     cycle += m_hashtable.insert(593);

//     cycle += m_hashtable.insert(999);
//     // will cross the board
//     // will evict 0
//     cycle += m_hashtable.insert(999);

//     // nothing happer
//     cycle += m_hashtable.insert(1);
//     // should evict 999
//     cycle += m_hashtable.insert(0);

//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }

//   SECTION("evict to cover self") {
//     // should be error
//   }
//   SECTION("evict to cover origin") {
//     // should be error, not simulated yet
//   }
//   SECTION("query_and_delete") {
//     auto cycle = 0;
//     cycle += m_hashtable.insert(1);
//     cycle += m_hashtable.insert(0);
//     cycle += m_hashtable.query_and_delete(1);

//     fmt::print("{}\n", m_hashtable.get_line_trace());
//   }
// }