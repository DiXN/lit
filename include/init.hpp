#pragma once

#include "arg.h"
#include "repository.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>

using namespace std;
namespace fs = std::filesystem;

class Init : public Arg {
  public:
  Init() {}
  bool invoke(const optional<string> arg) const override {
    if (arg) {
      cerr << "On command 'lit Init' no argument is allowed" << endl;
      return false;
    }

    const auto lit_path = fs::current_path() / ".lit";
    const auto& repo = Repository::instance();

    if (repo.exists())
      cout << "repository is already initialized." << endl;
    else {
      repo.initialize();
      cout << "repository is initialzed." << endl;
    }

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Initializes the \"lit\" repository." << endl;
    return os;
  }
};
