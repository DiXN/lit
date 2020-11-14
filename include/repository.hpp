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

  optional<string> find_branch_for_commit(const string& commit) const {
    for (auto& p: fs::directory_iterator(lit_path / "branches")) {
      const auto file_name = p.path().filename();

      if (file_name == ".total" || file_name == ".last")
        continue;

      ifstream branch_file(lit_path / "branches" / file_name);

      string line;
      string branch_commit;

      while(getline(branch_file, line)) {
        array<string, 3> tokens = extract_commit_information(line, '|');
        branch_commit = tokens[0];

        if(branch_commit == commit) {
          return optional<string>(file_name);
        }
      }
    }

    return nullopt;
  }

  void copy_structure(const string&& type) const {
    for (auto& p: fs::directory_iterator(root_path())) {
      if (!is_excluded(p)) {
        auto init_path = lit_path / "state" / type;

        fs::remove_all(init_path);
        fs::create_directory(init_path);

        try {
          fs::copy(p, init_path / fs::relative(p, root_path()));
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

  array<string, 3> extract_commit_information(const string& line, char delim) const {
    array<string, 3> tokens;
    stringstream line_stream(line);
    string token;

    for(int i = 0; getline(line_stream, token, delim); i++) {
      tokens[i] = token;
    }

    return tokens;
  }

  bool checkout_commit(const string& commit_id, optional<string> branch = nullopt, const string& patch_directory = ".", bool change_branch = true) const {
    for (auto& p: fs::directory_iterator(lit_path / "branches")) {
      if(p.path().filename() == ".last" || p.path().filename() == ".total") {
        continue;
      }

      cout << "looking for commit in " << p.path().filename() << endl;

      ifstream branch_file;

      if (!branch)
        branch_file.open(p.path());
      else
        branch_file.open(lit_path / "branches" / *branch);

      string commit;
      string line;
      vector<string> commits_in_branch;

      while(getline(branch_file, line)) {
        array<string, 3> tokens = extract_commit_information(line, '|');
        commit = tokens[0];
        commits_in_branch.push_back(commit);

        if (commit == commit_id) {
          if (change_branch) {
            switch_branch(p.path().filename());
            ofstream last(lit_path / "branches" / ".last");
            last << line;

            clean();
          }

          fs::path path_l = fs::path(patch_directory);

          int path_level = 0;

          while(path_l.has_parent_path() && path_l != path_l.root_path()) {
            path_l = path_l.parent_path();
            path_level++;
          }

          for (auto& com : commits_in_branch) {
            const auto& patch_file = lit_path / "objects" / com;

            if (patch_directory != ".") {
              auto command = Command("patch")
                              .arg(string("-s")).arg(string("-p") + to_string(path_level))
                              .arg(string("-d")).arg(patch_directory)
                              .arg(string("-i")).arg(patch_file);
              const auto& [output, status] = command.invoke();
            } else {
              auto command = Command("patch")
                              .arg(string("-s"))
                              .arg(string("-d")).arg(patch_directory)
                              .arg(string("-i")).arg(patch_file);
              const auto& [output, status] = command.invoke();
            }
          }

          if (change_branch)
            copy_structure(move("current"));

          return true;
        }
      }

      commits_in_branch.clear();
    }

    return false;
  }

  enum DiffTypes {
    modified,
    deleted,
    added,
    none
  };

  array<char, 4> diff_types_label {
    'M',
    'D',
    'A',
    '\0'
  };

  map<fs::path, DiffTypes> file_differences(fs::path init_path, fs::path compare_path) const {
    vector<fs::path> deleted_paths;
    map<fs::path, DiffTypes> file_diffs;

    for (auto& p: fs::recursive_directory_iterator(compare_path)) {
      const auto& curr_relative = fs::relative(p, compare_path);
      const auto& search_path = init_path / curr_relative;

      if(!fs::exists(search_path)) {
        file_diffs.emplace(curr_relative, DiffTypes::deleted);
        deleted_paths.push_back(p);
      }
    }

    for (auto& p: fs::recursive_directory_iterator(init_path)) {
      if (!is_excluded(p)) {
        const auto& curr_relative = fs::relative(p, init_path);
        const auto& search_path = compare_path / curr_relative;

        if(!fs::exists(search_path)) {
          file_diffs.emplace(curr_relative, DiffTypes::added);
        } else if (find(deleted_paths.begin(), deleted_paths.end(), p) == deleted_paths.end()) {
          auto command = Command("diff").arg(init_path / curr_relative).arg(search_path);
          const auto& [output, status] = command.invoke();

          if (status == 1)
            file_diffs.emplace(curr_relative.filename(), DiffTypes::modified);
        }
      }
    }

    return file_diffs;
  }
};

