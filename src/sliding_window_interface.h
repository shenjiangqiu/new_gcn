//
// Created by Jiangqiu shen on 5/14/21.
//

#ifndef GCN_SIM_SLIDING_WINDOW_INTERFACE_H
#define GCN_SIM_SLIDING_WINDOW_INTERFACE_H

#include "cstdint"
#include "vector"
#include <memory>
class sliding_window_interface {
public:
  virtual ~sliding_window_interface()=default;
  // setting the window size and location,x:begin of x, _xw:x width
  // _y:all rows
  virtual void set_location(unsigned x, unsigned _xw, std::vector<unsigned> _y,
                            unsigned _level) = 0;
  virtual void set_location(unsigned x, unsigned _xw, unsigned _y, unsigned yw,
                            unsigned _level) = 0;
  virtual void set_addr(std::vector<uint64_t> inputAddr, unsigned inputLen,
                        uint64_t edgeAddr, unsigned edgeLen,
                        uint64_t outputAddr, unsigned outputLen) = 0;
  virtual void set_addr(uint64_t inputAddr, unsigned inputLen,
                        uint64_t edgeAddr, unsigned edgeLen,
                        uint64_t outputAddr, unsigned outputLen) = 0;

  virtual void set_size(unsigned currentEdges, unsigned currentNodeSize) = 0;

  virtual void set_prop(bool the_final_col, bool theFinalRow, bool theFirstRow,
                        bool the_final_layer) = 0;

  [[nodiscard]] virtual unsigned getX() const = 0;

  [[nodiscard]] virtual std::vector<unsigned int> getY() const = 0;
  [[nodiscard]] virtual unsigned int getY_c() const = 0;
  [[nodiscard]] virtual unsigned getYw() const = 0;


  [[nodiscard]] virtual unsigned getXw() const = 0;

  [[nodiscard]] virtual unsigned getLevel() const = 0;

  [[nodiscard]] virtual std::vector<uint64_t> getInputAddr() const = 0;
  [[nodiscard]] virtual uint64_t getInputAddr_c() const = 0;

  [[nodiscard]] virtual uint64_t getEdgeAddr() const = 0;

  [[nodiscard]] virtual uint64_t getOutputAddr() const = 0;

  [[nodiscard]] virtual unsigned getInputLen() const = 0;

  [[nodiscard]] virtual unsigned getEdgeLen() const = 0;

  [[nodiscard]] virtual unsigned getOutputLen() const = 0;

  [[nodiscard]] virtual unsigned getCurrentNodeSize() const = 0;
  [[nodiscard]] virtual bool isTheFinalCol() const = 0;
  [[nodiscard]] virtual bool isTheFinalRow() const = 0;
  [[nodiscard]] virtual bool isTheFirstRow() const = 0;
  [[nodiscard]] virtual bool isTheFinalLayer() const = 0;
  virtual void setTheFinalRow(bool theFinalRow) = 0;

  [[nodiscard]] virtual unsigned getNumEdgesInWindow() const = 0;
};
using window_iter =
    std::vector<std::shared_ptr<sliding_window_interface>>::iterator;

#endif // GCN_SIM_SLIDING_WINDOW_INTERFACE_H
