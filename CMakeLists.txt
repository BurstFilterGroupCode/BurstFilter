cmake_minimum_required(VERSION 3.16)
project(newsketch)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

add_compile_options("-Wall")
add_compile_options("-O2")


add_library(dataset src/util/read_dataset.cpp)

add_library(correct src/read_correct.cpp)
add_library(hash src/util/BOBHash32.cpp)
add_library(param src/sketch/newsketch/param.cpp)
add_library(time src/util/time.cpp)

add_executable(run_newsketch src/run_newsketch.cpp)
target_link_libraries(run_newsketch PUBLIC dataset)
target_link_libraries(run_newsketch PUBLIC correct)
target_link_libraries(run_newsketch PUBLIC hash)
target_link_libraries(run_newsketch PUBLIC param)
target_link_libraries(run_newsketch PUBLIC time)

add_executable(run_correct src/run_correct.cpp)
target_link_libraries(run_correct PUBLIC dataset)
target_link_libraries(run_correct PUBLIC hash)
target_link_libraries(run_correct PUBLIC param)
target_link_libraries(run_correct PUBLIC time)

add_executable(run_burstyevent src/run_burstyevent.cpp)
target_link_libraries(run_burstyevent PUBLIC dataset)
target_link_libraries(run_burstyevent PUBLIC correct)
target_link_libraries(run_burstyevent PUBLIC hash)
target_link_libraries(run_burstyevent PUBLIC param)
target_link_libraries(run_burstyevent PUBLIC time)

add_executable(run_topicsketch src/run_topicsketch.cpp)
target_link_libraries(run_topicsketch PUBLIC dataset)
target_link_libraries(run_topicsketch PUBLIC correct)
target_link_libraries(run_topicsketch PUBLIC hash)
target_link_libraries(run_topicsketch PUBLIC param)
target_link_libraries(run_topicsketch PUBLIC time)

add_executable(run_burstsketch src/run_burstsketch.cpp)
target_link_libraries(run_burstsketch PUBLIC dataset)
target_link_libraries(run_burstsketch PUBLIC correct)
target_link_libraries(run_burstsketch PUBLIC hash)
target_link_libraries(run_burstsketch PUBLIC param)
target_link_libraries(run_burstsketch PUBLIC time)

add_executable(chk_cache src/chk_cache.cpp)

add_executable(chk_lru src/chk_lru.cpp)
target_link_libraries(chk_lru PUBLIC dataset)
target_link_libraries(chk_lru PUBLIC hash)

add_executable(chk_lfu src/chk_lfu.cpp)
target_link_libraries(chk_lfu PUBLIC dataset)
target_link_libraries(chk_lfu PUBLIC hash)

add_executable(runtest runtest/runtest.cpp)

add_executable(run_timewidth_test src/run_timewidth_test.cpp src/sketch/other_test/param.cpp)
target_link_libraries(run_timewidth_test PUBLIC hash)
target_link_libraries(run_timewidth_test PUBLIC dataset)

add_executable(run_strawman src/run_strawman.cpp)
target_link_libraries(run_strawman PUBLIC dataset)
target_link_libraries(run_strawman PUBLIC correct)
target_link_libraries(run_strawman PUBLIC hash)
target_link_libraries(run_strawman PUBLIC param)
target_link_libraries(run_strawman PUBLIC time)

add_executable(run_temp src/run_temp.cpp)
target_link_libraries(run_temp PUBLIC dataset)
target_link_libraries(run_temp PUBLIC hash)

add_executable(run_jitter_test src/run_jitter_test.cpp)
target_link_libraries(run_jitter_test PUBLIC dataset)
target_link_libraries(run_jitter_test PUBLIC hash)