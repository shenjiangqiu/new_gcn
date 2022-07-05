//
// Created by Jiangqiu shen on 5/10/21.
//
#include "boost/range/irange.hpp"
#include <catch2/catch_test_macros.hpp>

#include "fmt/format.h"
#include "globals.h"
#include "memory_interface.h"
#include "spdlog/spdlog.h"
using namespace boost;
TEST_CASE("mem_interface_test") {
  char *argv[2];
  char arg1[100] = "-mem-sim=dramsim3";
  argv[0] = arg1;
  argv[1] = arg1;
  int argc = 2;
  Minisat::parseOptions(argc, argv, false);
  std::string config_name = "HBM2_8Gb_x128.ini";
  memory_interface m_mem_interface(config_name, config_name, 10);
  spdlog::set_level(spdlog::level::debug);
  SECTION("test_single") {
    for (auto i : irange(10)) {

      auto req = std::make_shared<Req>();
      req->set_addr(i * 32 + 1, 64);
      m_mem_interface.send(req);
    }
    while (!m_mem_interface.empty()) {
      while (!m_mem_interface.ret_available()) {
        m_mem_interface.cycle();
      }
      auto ret = m_mem_interface.get_req();
      fmt::print("{}\n", *ret);
    }
  }
  SECTION("test_sep") {
    auto req = std::make_shared<Req>();
    req->set_addr({1, 100, 1000, 200, 32});
    m_mem_interface.send(req);
    auto req2 = std::make_shared<Req>();
    req2->set_addr({44, 77, 88, 123, 500});
    m_mem_interface.send(req2);
    auto req3 = std::make_shared<Req>();
    req3->set_addr({1002, 500});
    m_mem_interface.send(req3);

    while (!m_mem_interface.empty()) {
      while (!m_mem_interface.ret_available()) {
        m_mem_interface.cycle();
      }
      auto ret = m_mem_interface.get_req();
      m_mem_interface.cycle();

      fmt::print("{}\n", *ret);
    }
  }
}