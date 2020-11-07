#pragma once

#include "repository.hpp"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <filesystem>
#include <tuple>

using namespace std;
namespace fs = std::filesystem;

class Merge: public Arg {
  private:
  tuple<bool, fs::path> prepare_merge() const {
    const auto& temp_path = fs::temp_directory_path();
    const auto& lit_merge_path = temp_path / "lit" / "merge";

    fs::remove_all(lit_merge_path);
    return make_tuple(fs::create_directories(lit_merge_path), lit_merge_path);
  }

  public:
  Merge() {}
  bool invoke(const optional<string> arg) const override {
    if (!arg) {
      cerr << "On command 'lit merge' an argument is required" << endl;
      return false;
    } else {
      const auto& [temp_path_success, lit_merge_path] = prepare_merge();
      if(!temp_path_success) {
        cerr << "Could not create temporary directory, that is necessary for merge" << endl;
        return false;
      }
    }

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Merges two \"lit\" commits." << endl;
    return os;
  }
};

