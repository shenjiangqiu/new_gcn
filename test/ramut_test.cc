//
// Created by sjq on 1/7/21.
//

#include<catch.hpp>
#include<ramulator_wrapper.h>
#include<string>
TEST_CASE("ramu_test"){
std::string   config("HBM-config.cfg");   //config("HBM2_8Gb_x128.ini");
ramulator_wrapper m_ramu(config,64);
int i=0;
int cycle=0;
int on_going=0;
while(i<1000000){
if(m_ramu.available()){
m_ramu.send(i*64,false);//only read
i++;
on_going++;
}
m_ramu.cycle();
cycle++;
if(m_ramu.return_available()){
m_ramu.pop();
on_going--;
}
}
while(on_going>0){
m_ramu.cycle();
cycle++;
if(m_ramu.return_available()){
m_ramu.pop();
on_going--;
}
}


}