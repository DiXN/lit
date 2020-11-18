#pragma once

#include <optional>
#include <sstream>

namespace lit {

using namespace std;

class Arg {
  public:
  virtual bool invoke(const optional<string> arg = nullopt) const = 0;
  virtual ostringstream info() const = 0;
};

}; // namespace lit
