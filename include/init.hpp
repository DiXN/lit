#pragma once

#include <arg.h>
#include <sstream>
#include <iostream>

using namespace std;

class Init: public Arg {
  public:
  Init() {}
  void invoke() const override {
    return;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Initializes the \"lit\" repository." << endl;
    return os;
  }
};

std::ostream& operator<<(std::ostream &strm, const Init& init) {
  return strm << "Init";
}

