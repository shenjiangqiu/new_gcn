//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_SYSTOLICARRAY_H
#define GCN_SIM_SYSTOLICARRAY_H
#include "Slide_window.h"
#include "buffer.h"
#include <memory>
class SystolicArray {
public:
  SystolicArray(int totalRows, int totalCols,
                const shared_ptr<Aggregator_buffer> &aggBuffer,
                const shared_ptr<WriteBuffer> &outputBuffer);
  void cycle();

private:
  int total_rows;
  int total_cols;
  std::shared_ptr<Slide_window> current_sliding_window;
  std::shared_ptr<Aggregator_buffer> agg_buffer;
  std::shared_ptr<WriteBuffer> output_buffer;

  int cal_remaining_cycle();
  bool empty{true};
  bool running{false};
  bool finished{false};
  int remaining_cycle{0};
};

#endif // GCN_SIM_SYSTOLICARRAY_H
