#pragma once

#include "repository.hpp"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

class Show: public Arg {
  public:
  Show() {}
  bool invoke(const optional<string> arg) const override {
    if (!arg) {
      cerr << "On command 'lit Show' an argument is required" << endl;
      return false;
    }

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Shows information of the specified commit." << endl;
    return os;
  }
};

