cmake_minimum_required(VERSION 3.16)

project(gcn_sim VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)


add_subdirectory(deps/fmt)
add_subdirectory(deps/ramulator)
add_subdirectory(deps/spdlog)
add_subdirectory(deps/minisat_utils)

file(GLOB lib_srcs src/*.cc)
list(FILTER lib_srcs EXCLUDE REGEX ".*main.cc")

add_library(gcn_sim_lib ${lib_srcs})
target_include_directories(gcn_sim_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(gcn_sim_lib PUBLIC ramulator fmt spdlog minisat_utiles)

add_executable(gcn_sim ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)

target_link_libraries(gcn_sim PUBLIC gcn_sim_lib fmt)


aux_source_directory(test TEST_SOURCE)
add_executable(gcn_sim_test ${TEST_SOURCE})
target_include_directories(gcn_sim_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/test)
target_link_libraries(gcn_sim_test PUBLIC gcn_sim_lib fmt)
configure_file(test/test.graph test.graph COPYONLY)

macro(copy_files GLOBPAT DESTINATION)
    file(GLOB COPY_FILES
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            ${GLOBPAT})
    add_custom_target(copy ALL
            COMMENT "Copying files: ${GLOBPAT}")

    foreach (FILENAME ${COPY_FILES})
        set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
        set(DST "${DESTINATION}/${FILENAME}")

        add_custom_command(
                TARGET copy
                COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
        )
    endforeach (FILENAME)
endmacro(copy_files)

file(GLOB GRAPHS data/*)

copy_files(data/* ${CMAKE_CURRENT_BINARY_DIR})