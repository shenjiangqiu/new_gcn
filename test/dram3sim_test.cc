//
// Created by by Jianhui on 3/22/21.
//

#include "catch2/catch.hpp"

#include <dramsim_wrapper.h>
#include <string>
TEST_CASE("dramsim3_test") {
  std::string config("HBM2_8Gb_x128.ini"); // config("HBM-config.cfg");   //
  dramsim_wrapper m_mem(config);
  int i = 0;
  int cycle = 0;
  int on_going = 0;
  int req_cnt = 1000000;
  int upper = 8;
  while (i < req_cnt) {

    for (int p = 0; p < upper; p++)
      if (m_mem.available(i * 64)) {
        m_mem.send(i * 64, false); // only read
        i++;
        on_going++;
      }

    m_mem.cycle();
    cycle++;

    for (int p = 0; p < upper; p++)
      if (m_mem.return_available()) {
        m_mem.pop();
        on_going--;
      }
  }

  while (on_going > 0) {
    m_mem.cycle();
    cycle++;
    if (m_mem.return_available()) {
      m_mem.pop();
      on_going--;
    }
  }

  std::cout << "ramu_test BW " << (double)64 * req_cnt / (double)cycle
            << " cycle " << cycle << "\n";
}