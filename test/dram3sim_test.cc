//
// Created by by Jianhui on 3/22/21.
//

#include<catch.hpp>
#include<dramsim_wrapper.h>
#include<string>
TEST_CASE("dramsim3_test"){
std::string  config("HBM2_8Gb_x128.ini");  // config("HBM-config.cfg");   //
dramsim_wrapper m_mem(config);
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