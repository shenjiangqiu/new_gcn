//
// Created by Jiangqiu shen on 1/19/21.
//

#ifndef GCN_SIM_TRANSFORMER_H
#define GCN_SIM_TRANSFORMER_H
#include "string"
#include <fstream>
#include <iostream>

class transformer {
public:
  transformer(std::string  input_name, std::string  output_name);
  void translate();
private:
  std::string input_name;
  std::string output_name;
};

#endif // GCN_SIM_TRANSFORMER_H
