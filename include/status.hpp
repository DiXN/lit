#pragma once

#include "repository.hpp"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

class Status: public Arg {
  public:
  Status() {}
  bool invoke(const optional<string> arg) const override {
    if (arg) {
      cerr << "On command 'lit status' no argument is allowed" << endl;
      return false;
    }

    const auto& repo = Repository::instance();
    const auto& last_commit = repo.last_commit();

    if (!last_commit) {
      for (auto& p: fs::recursive_directory_iterator(repo.root_path())) {
        if (!repo.is_excluded(p)) {
          cout << "A  " << fs::relative(p, repo.root_path()) << endl;
        }
      }
    }

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Shows the status of the \"lit\" repository." << endl;
    return os;
  }
};

