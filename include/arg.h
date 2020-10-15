#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

class Arg {
  public:
  virtual void invoke() const = 0;
  virtual ostringstream info() const = 0;
};


