#pragma once

#include "singleton.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <optional>
#include <string>
#include <algorithm>
#include <tuple>
#include <map>

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
      array<string, 4> tokens = extract_commit_information(line, '|');
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

    const auto& [last_commit_nr, date, message, parents] = *last_commit;

    stringstream prev_revision(last_commit_nr);
    char prev_rev;
    int prev_rev_number;
    prev_revision >> prev_rev;
    prev_revision >> prev_rev_number;

    return ++prev_rev_number;
  }

  bool is_head(const string& last_commit_nr) const {
    const auto& last_commit = last_commit_of_branch(current_branch());

    if(!last_commit) {
      return false;
    }

    const auto &[current_commit_nr, current_date, current_message, parents] = *last_commit;

    if (current_commit_nr != last_commit_nr) {
      return false;
    } else {
      return true;
    }

    return false;
  }

  optional<tuple<string, string, string, string>> last_commit_of_branch(const string& branch) const {
    ifstream branch_file(lit_path / "branches" / branch);

    string commit;
    string time;
    string message;
    string line;
    string parents;

    while(getline(branch_file, line)) {
      array<string, 4> tokens = extract_commit_information(line, '|');
      commit = tokens[0];
      time = tokens[1];
      message = tokens[2];
      parents = tokens[3];
    }

    if (commit.empty()) {
      return nullopt;
    } else {
      return optional(make_tuple(commit, time, message, parents));
    }
  }

  optional<vector<string>> extract_parents(const string& parents) const {
    if (parents == "root")
      return nullopt;

    const auto token_position = parents.find_first_of(",");

    if (token_position == string::npos)
      return optional(vector { parents });

    const auto first_parent = parents.substr(0, token_position);
    const auto second_parent = parents.substr(token_position + 1, parents.length());

    return optional(vector { first_parent, second_parent });
  }

  void copy_structure(const string&& type) const {
    auto init_path = lit_path / "state" / type;
    fs::remove_all(init_path);
    fs::create_directory(init_path);

    for (auto& p: fs::directory_iterator(root_path())) {
      if (!is_excluded(p)) {
        try {
          fs::copy(p, init_path / p.path().filename(), fs::copy_options::recursive);
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

  optional<string> find_base_commit(const string& branch_a, const string& branch_b) const {
    string branch_a_line;
    string branch_b_line;

    const auto& branch_location = lit_path / "branches";
    ifstream branch_a_file(branch_location / branch_a);
    ifstream branch_b_file(branch_location / branch_b);

    vector<optional<vector<string>>> branch_a_lines;
    vector<optional<vector<string>>> branch_b_lines;

    while(getline(branch_a_file, branch_a_line)) {
      array<string, 4> tokens = extract_commit_information(branch_a_line, '|');
      const auto& parents = extract_parents(tokens[3]);
      branch_a_lines.push_back(parents);
    }

    while(getline(branch_b_file, branch_b_line)) {
      array<string, 4> tokens = extract_commit_information(branch_b_line, '|');
      const auto& parents = extract_parents(tokens[3]);
      branch_b_lines.push_back(parents);
    }

    for(auto it_a = branch_a_lines.rbegin(); it_a != branch_a_lines.rend(); ++it_a) {
      for(auto it_b = branch_b_lines.rbegin(); it_b != branch_b_lines.rend(); ++it_b) {
        if(*it_a && *it_b) {
          for(const auto& elem : **it_a) {
            if (std::find((**it_b).cbegin(), (**it_b).cend(), elem) != (**it_b).cend())
              return optional(elem);
          }
        }
      }
    }

    return nullopt;
  }

  array<string, 4> extract_commit_information(const string& line, char delim) const {
    array<string, 4> tokens;
    stringstream line_stream(line);
    string token;

    for(int i = 0; getline(line_stream, token, delim); i++) {
      tokens[i] = token;
    }

    return tokens;
  }
};

