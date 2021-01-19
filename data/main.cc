//
// Created by Jiangqiu shen on 1/19/21.
//
#include "transformer.h"
#include <iostream>
int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "wrong argc" << std::endl;
    return -1;
  }

  transformer m_transformer(argv[1], argv[2]);
  m_transformer.translate();
}