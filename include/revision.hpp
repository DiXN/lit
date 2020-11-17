#pragma once

#include "repository.hpp"

#include <sstream>
#include <iostream>
#include <filesystem>
#include <ctime>

using namespace std;

class Revision {
  private:
    string _revision;
    string _message;
    string _date;
    optional<vector<string>> _parents;
    string _parents_str;

    const Repository& repo = Repository::instance();

  public:
  Revision(const string& revision) {
    ifstream branch_file(repo.get_lit_path() / "branches" / ".total");

    string line;
    string branch_commit;

    while(getline(branch_file, line)) {
      array<string, 4> tokens = repo.extract_commit_information(line, '|');
      branch_commit = tokens[0];

      if(branch_commit == revision) {
        _revision = branch_commit;
        _date = tokens[1];
        _message = tokens[2];

        const auto& parents = repo.extract_parents(tokens[3]);

        if (parents)
          _parents = parents;
      }
    }
  }

  Revision(const string& message, const string& parents, const int revision_id = -1) {
    _parents = repo.extract_parents(parents);
    _message = message;

    if (revision_id > - 1) {
      stringstream oss;
      oss << "r" << revision_id;
      _revision = oss.str();
    } else {
      stringstream oss;
      oss << "r" << repo.unique_commit_id();
      _revision = oss.str();
    }

    const auto now = std::time(nullptr);
    stringstream time_stream;
    time_stream << put_time(localtime(&now), "%c");
    _date = time_stream.str();

    _parents_str = parents;
  }

  string message() const {
    return _message;
  }

  string date() const {
    return _date;
  }

  optional<vector<string>> parents() const {
    return _parents;
  }

  string revision() const {
    return _revision;
  }

  void write(const string& branch) const {
    stringstream new_revision;
    new_revision << _revision << "|" << _date << "|" << _message << "|" << _parents_str << endl;
    repo.write_commit(branch, new_revision);
  }

  static bool checkout_revision(const string& commit_id, optional<string> branch = nullopt, const string& patch_directory = ".", bool change_branch = true)  {
    const auto& repo = Repository::instance();
    const auto lit_path = repo.get_lit_path();

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
        array<string, 4> tokens = repo.extract_commit_information(line, '|');
        commit = tokens[0];
        commits_in_branch.push_back(commit);

        if (commit == commit_id) {
          if (change_branch) {
            repo.switch_branch(p.path().filename());
            ofstream last(lit_path / "branches" / ".last");
            last << line;

            repo.clean();
          }

          fs::path path_l;
          if (patch_directory == ".")
            path_l = fs::path(repo.root_path());
          else
            path_l = fs::path(patch_directory);

          int path_level = 0;

          while(path_l.has_parent_path() && path_l != path_l.root_path()) {
            path_l = path_l.parent_path();
            path_level++;
          }

          for (auto& com : commits_in_branch) {
            const auto& patch_file = repo.get_lit_path() / "objects" / com;

            if (patch_directory != ".") {
              auto command = Command("patch")
                              .arg(string("-s")).arg(string("-p") + to_string(path_level))
                              .arg(string("-d")).arg(patch_directory)
                              .arg(string("-i")).arg(patch_file);
              const auto& [output, status] = command.invoke();
            } else {
              auto command = Command("patch")
                              .arg(string("-s")).arg(string("-p") + to_string(path_level + 1))
                              .arg(string("-d")).arg(patch_directory)
                              .arg(string("-i")).arg(patch_file);
              const auto& [output, status] = command.invoke();
            }
          }

          if (change_branch)
            repo.copy_structure(move("current"));

          return true;
        }
      }

      commits_in_branch.clear();
    }

    return false;
  }
};
