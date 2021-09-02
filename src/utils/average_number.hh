#ifndef AVERAGE_NUMBER_HH
#define AVERAGE_NUMBER_HH

class average_number {

public:
  average_number() : number(0), current_count(0) {}
  void update(double new_nuber) {
    number += new_nuber;
    current_count++;
  }
  double get_average() const { return number / (double)current_count; }

private:
  double number;
  unsigned long long current_count;
};

#endif