#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <iostream>
#include <optional>

using namespace std;

class Arg {
  public:
  virtual bool invoke(const optional<string> arg = nullopt) const = 0;
  virtual ostringstream info() const = 0;
};


