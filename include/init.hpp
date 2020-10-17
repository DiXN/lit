#pragma once

#include "repository.hpp"
#include <arg.h>
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

class Init: public Arg {
  public:
  Init() {}
  void invoke() const override {
    const auto lit_path = fs::current_path() / ".lit";
    const auto& repo = Repository::instance();

    if (repo.exists())
      cout << "repository is already initialized." << endl;
    else {
      repo.initialize();
      cout << "repository is initialzed." << endl;
    }
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Initializes the \"lit\" repository." << endl;
    return os;
  }
};

