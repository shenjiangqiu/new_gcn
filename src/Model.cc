//
// Created by Jiangqiu shen on 1/18/21.
//

#include "Model.h"

#include <utility>
Model::Model(std::string name, std::vector<int> mLevels, bool isConcatenate,
             bool isSample, int sampleNum)
    : name(std::move(name)), m_levels(std::move(mLevels)), is_concatenate(isConcatenate), is_sample(isSample),
      sample_num(sampleNum) {}
const std::string &Model::getName() const { return name; }
const std::vector<int> &Model::getMLevels() const { return m_levels; }
bool Model::isConcatenate() const { return is_concatenate; }
