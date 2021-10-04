cmake_minimum_required(VERSION 3.16)

project(gcn_sim VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)
find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)


option(THERMAL "enable thermal in dramsim" OFF)
option(USEDRAMSIM "enable dramsim" ON)

add_subdirectory(deps/ramulator)
add_subdirectory(deps/minisat_utils)
add_subdirectory(data)

message("use thermal:" ${THERMAL})
message("use dramsim:" ${USEDRAMSIM})
if(USEDRAMSIM)
add_subdirectory(deps/DRAMsim3)
endif(USEDRAMSIM)

add_subdirectory(deps/HBM)

add_compile_options("-Wall")
file(GLOB lib_srcs src/*.cc)
list(FILTER lib_srcs EXCLUDE REGEX ".*main.cc")

add_library(gcn_sim_lib ${lib_srcs} src/dramsim_wrapper.cc src/dramsim_wrapper.h)
target_include_directories(gcn_sim_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(USEDRAMSIM)
target_link_libraries(gcn_sim_lib PUBLIC dramsim3)
target_compile_definitions(gcn_sim_lib PUBLIC  USEDRAM3)
endif()

target_link_libraries(gcn_sim_lib PUBLIC ramulator fmt::fmt-header-only  minisat_utiles dramsim2)
target_link_libraries(gcn_sim_lib PUBLIC spdlog::spdlog_header_only)
target_compile_options(gcn_sim_lib PRIVATE -Wall -Wextra)
add_executable(gcn_sim ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)

target_link_libraries(gcn_sim PUBLIC gcn_sim_lib fmt::fmt-header-only)


aux_source_directory(test TEST_SOURCE)
add_executable(gcn_sim_test ${TEST_SOURCE} test/mapping_test.cc)
target_include_directories(gcn_sim_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/test)
target_link_libraries(gcn_sim_test PUBLIC gcn_sim_lib fmt::fmt-header-only Catch2::Catch2)
configure_file(test/test.graph test.graph COPYONLY)

configure_file(data/cora.graph cora.graph COPYONLY)
configure_file(data/pubmed.graph pubmed.graph COPYONLY)
configure_file(data/citeseer.graph citeseer.graph COPYONLY)
configure_file(deps/ramulator/configs/DDR4-config.cfg DDR4-config.cfg COPYONLY)
configure_file(deps/ramulator/configs/HBM-config.cfg HBM-config.cfg COPYONLY)
