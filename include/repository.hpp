#pragma once

#include "singleton.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <algorithm>
#include <tuple>

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
    const auto state_path = lit_path / "state";
    const auto init_state_path = lit_path / "state" / "init";
    const auto current_state_path = lit_path / "state" / "current";
    fs::create_directory(lit_path);
    fs::create_directory(ref_path);
    fs::create_directory(current_ref);
    fs::create_directory(object_path);
    fs::create_directory(branch_path);
    fs::create_directory(state_path);
    fs::create_directory(init_state_path);
    fs::create_directory(current_state_path);
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

  array<string, 3> extract_commit_information(const string& line, char delim) const {
    array<string, 3> tokens;
    stringstream line_stream(line);
    string token;

    for(int i = 0; getline(line_stream, token, delim); i++) {
      tokens[i] = token;
    }

    return tokens;
  }

  optional<tuple<string, string, string>> last_commit_of_branch(const string& branch) const {
    ifstream branch_file(lit_path / "branches" / branch);

    string commit;
    string time;
    string message;
    string line;

    while(getline(branch_file, line)) {
      array<string, 3> tokens = extract_commit_information(line, '|');
      commit = tokens[0];
      time = tokens[1];
      message = tokens[2];
    }

    if (commit.empty()) {
      return nullopt;
    } else {
      return optional<tuple<string, string, string>>(make_tuple(commit, time, message));
    }
  }

  void copy_structure(const string type) const {
    for (auto& p: fs::directory_iterator(root_path())) {
      if (!is_excluded(p)) {
        auto init_path = lit_path / "state" / type;
        const auto copy_options = fs::copy_options::update_existing | fs::copy_options::recursive;

        try {
          fs::copy(p, init_path / fs::relative(p, root_path()), copy_options);
        } catch(fs::filesystem_error& er) {
          cerr << "copy failed: " << er.what() << endl;
        }
      }
    }
  }

  void clean() const {
    for (auto& p: fs::directory_iterator(root_path())) {
      if (!is_excluded(p)) {
        fs::remove_all(p);
      }
    }
  }
};

