#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <vector>

using namespace std;

class Arg {
  public:
  virtual bool invoke(const optional<string> arg = nullopt) const = 0;
  virtual ostringstream info() const = 0;
};
