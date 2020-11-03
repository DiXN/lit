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
    ofstream total(branch_path / ".total");
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

  void create_branch(const string& branch) const {
    ofstream new_branch(lit_path / "branches" / branch);
  }

  void switch_branch(const string& branch) const {
    const auto& branch_ref_root = lit_path / "ref";
    fs::remove(branch_ref_root / current_branch());
    fs::create_directory(branch_ref_root / branch);
  }

  void copy_commit_structure(const string& new_branch, const string& previous_branch) const {
    ifstream branch_file(lit_path / "branches" / previous_branch);

    ofstream new_branch_file;
    new_branch_file.open(lit_path / "branches" / new_branch, ofstream::out | ofstream::app);

    string commit;
    string line;

    while(getline(branch_file, line)) {
      array<string, 3> tokens = extract_commit_information(line, '|');
      commit = tokens[0];

      new_branch_file << line << endl;

      if (commit == new_branch) {
        break;
      }
    }
  }

  int unique_commit_id() const {
    const auto& last_commit = last_commit_of_branch(".total");

    if (!last_commit) {
      return 0;
    }

    const auto& [last_commit_nr, date, message] = *last_commit;

    stringstream prev_revision(last_commit_nr);
    char prev_rev;
    int prev_rev_number;
    prev_revision >> prev_rev;
    prev_revision >> prev_rev_number;

    return ++prev_rev_number;
  }

  void write_commit(const string& branch, const stringstream& ss) const {
    ofstream commit;
    commit.open(lit_path / "branches" / branch, ofstream::out | ofstream::app);
    commit << ss.str();
    commit.close();

    ofstream last(lit_path / "branches" / ".last");
    last << ss.str();
    last.close();

    cout << ss.str();

    ofstream total;
    total.open(lit_path / "branches" / ".total", ofstream::out | ofstream::app);
    total << ss.str();
    total.close();
  }

  bool is_head(const string& last_commit_nr) const {
    const auto& last_commit = last_commit_of_branch(current_branch());

    if(!last_commit) {
      return false;
    }

    const auto &[current_commit_nr, current_date, current_message] = *last_commit;

    if (current_commit_nr != last_commit_nr) {
      return false;
    } else {
      return true;
    }

    return false;
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

