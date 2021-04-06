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
  Model(std::string name, std::vector<int> mLevels, bool isConcatenate, bool isInitialResidual=false,
        bool isSample = false, int sampleNum = 0);
  Model(const Model &other) = default;
  Model(Model &&other) = default;
  Model() = default;
  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::vector<int> &getMLevels() const;
  [[nodiscard]] bool isConcatenate() const;
  [[nodiscard]] bool isInitialResidual() const;


private:
  std::string name;
  std::vector<int> m_levels;
//  int sample_num;
//  bool is_sample;
  bool is_concatenate;
  bool is_initial_residual;
};

#endif // GCN_SIM_MODEL_H
