#pragma once

#include "singleton.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <algorithm>

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
    fs::create_directory(lit_path);
    fs::create_directory(ref_path);
    fs::create_directory(current_ref);
    fs::create_directory(object_path);
    fs::create_directory(branch_path);
  };

  bool exists() const {
    if (fs::exists(lit_path)) {
      return true;
    } else {
      return false;
    }
  }

  fs::path get_lit_path() const {
    return lit_path;
  }

  fs::path root_path() const {
    return lit_path.parent_path();
  }

  bool is_excluded(fs::path path) const {
    const auto& git_path = root_path() / ".git";

    if (search(path.begin(), path.end(), git_path.begin(), git_path.end()) != path.end() ||
          search(path.begin(), path.end(), lit_path.begin(), lit_path.end()) != path.end()) {
      return true;
    } else {
      return false;
    }
  }

  string current_branch() const {
    const auto branch_ref_path = lit_path / "ref";

    fs::path current_branch;
    for(auto& branch_ref: fs::directory_iterator(branch_ref_path)) {
      current_branch = fs::relative(branch_ref, branch_ref_path);
      break;
    }

    return current_branch.string();
  }

  optional<string> last_commit() const {
    ifstream branch_file(lit_path / "branches" / current_branch());
    string commit;
    while(branch_file >> commit);

    if (commit.empty()) {
      return nullopt;
    } else {
      return optional<string>(commit);
    }
  }
};

