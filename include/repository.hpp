#pragma once

#include "singleton.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

using namespace std;
namespace fs = std::filesystem;

class Repository: public Singleton<Repository> {
  private:
  fs::path lit_path;

  public:
  Repository(const fs::path lit_path = fs::current_path() / ".lit") : lit_path(lit_path) { }

  void initialize() const {
    const auto ref_path = lit_path / "ref";
    const auto current_ref = ref_path / "master";
    const auto object_path = lit_path / "objects";
    const auto branch_path = lit_path / "branches";
    const auto master_branch = branch_path / "master.txt";
    fs::create_directory(lit_path);
    fs::create_directory(ref_path);
    fs::create_directory(current_ref);
    fs::create_directory(object_path);
    fs::create_directory(branch_path);
    ofstream (master_branch.string());
  };

  bool exists() const {
    if (fs::exists(lit_path)) {
      return true;
    } else {
      return false;
    }
  }

  optional<string> last_commit() const {
    const auto branch_ref_path = lit_path / "ref";

    for(auto& branch_ref: fs::directory_iterator(branch_ref_path)) {
      const auto current_branch = fs::relative(branch_ref, branch_ref_path);
      ifstream branch_file(lit_path / "branches" / current_branch);
      string commit;
      branch_file >> commit;

      if (commit.empty()) {
        return nullopt;
      } else {
        return optional<string>(commit);
      }
    }

    return nullopt;
  }
};

