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
  Model(std::string name, std::vector<int> mLevels, bool isConcatenate,
        bool isSample=false, int sampleNum=0);
  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::vector<int> &getMLevels() const;
  [[nodiscard]] bool isConcatenate() const;

private:
  std::string name;
  std::vector<int> m_levels;
  int sample_num;
  bool is_sample;
  bool is_concatenate;
};

#endif // GCN_SIM_MODEL_H
