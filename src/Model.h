//
// Created by Jiangqiu shen on 1/18/21.
//

#ifndef GCN_SIM_MODEL_H
#define GCN_SIM_MODEL_H
#include "graph.h"
#include "string"
#include "vector"
class Model {
public:
  Model(std::string name, std::vector<int> mLevels,
         bool isConcatenate);
  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::vector<int> &getMLevels() const;
  [[nodiscard]] bool isConcatenate() const;

private:
  std::string name;
  std::vector<int> m_levels;
  bool is_concatenate;
};

#endif // GCN_SIM_MODEL_H
