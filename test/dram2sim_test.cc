//
// Created by Jianhui on 3/22/21.
//

#include<catch.hpp>
#include<dramsim2_wrapper.h>
#include<string>
TEST_CASE("dramsim2_test"){
std::string  dev_cfg("HBMDevice4GbLegacy.ini");  
std::string  sys_cfg("HBMSystemLegacy.ini");

dramsim2_wrapper m_mem(sys_cfg,dev_cfg);
int i=0;
int cycle=0;
int on_going=0;
while(i<1000000){
if(m_mem.available()){
    m_mem.send(i*64,false);//only read
    i++;
    on_going++;
}
m_mem.cycle();
cycle++;
if(m_mem.return_available()){
m_mem.pop();
on_going--;
}
}
while(on_going>0){
    m_mem.cycle();
    cycle++;
    if(m_mem.return_available()){
        m_mem.pop();
        on_going--;
    }
}


}