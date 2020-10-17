#pragma once

#include "singleton.h"
#include <filesystem>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

class Repository: public Singleton<Repository> {
  private:
  fs::path lit_path;

  public:
  Repository(const fs::path lit_path = fs::current_path() / ".lit") : lit_path(lit_path) { }

  void initialize() const {
    const auto object_path = lit_path / "objects";
    const auto branch_path = lit_path / "branches";
    const auto master_branch = branch_path / "master";
    fs::create_directory(lit_path);
    fs::create_directory(object_path);
    fs::create_directory(branch_path);
    fs::create_directory(master_branch);
  };

  bool exists() const {
    if (fs::exists(lit_path)) {
      return true;
    } else {
      return false;
    }
  }
};

