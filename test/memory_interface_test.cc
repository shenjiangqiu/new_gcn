//
// Created by sjq on 1/7/21.
//

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <fmt/format.h>
#include <memory_interface.h>

TEST_CASE("mem_interface") {
  memory_interface m_interface("DDR4-config.cfg", "dev-fie.cfg", 128);
  uint64_t cycle = 0;
  for (auto i = 0; i < 100; i++) {
    auto req = std::make_shared<Req>();

    req->req_type = mem_request::read;
    req->set_addr(std::vector<uint64_t>{(uint64_t)(i)*2048});
    req->t = device_types::input_buffer;
    while (!m_interface.available()) {
      m_interface.cycle();
      cycle++;
    }
    m_interface.send(req);
  }
  for (auto i = 0; i < 100; i++) {
    while (!m_interface.ret_available()) {
      m_interface.cycle();
      cycle++;
      // std::cout << cycle << std::endl;
    }
    auto ret = m_interface.get_req();
    std::cout << fmt::format("receive id: {} ,addr: {} \n", ret->id,
                             fmt::join(ret->get_addr(), ","));
  }
  REQUIRE(m_interface.empty() == true);
  std::cout << fmt::format("total cycle: {}\n", cycle);
}