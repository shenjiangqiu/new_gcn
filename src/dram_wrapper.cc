//
// Created by Jiangqiu shen on 1/15/21.
// this is a interface page for the dram wrapper
//

#include "dram_wrapper.h"

dram_wrapper::dram_wrapper( ){
    
    for(int i = 0; i <512; i++)
       bank_req_cnt[i] = 0;

     bank_infligt_req_cnt = 0;   
     sum_inflight_bank_req = 0;
}