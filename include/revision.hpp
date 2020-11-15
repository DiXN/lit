#pragma once

#include "repository.hpp"

#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;

class Revision {
  private:
    string _revision;
    string _message;
    string _date;
    optional<vector<string>> _parents;

  public:
  Revision(const string& revision) {
    const auto& repo = Repository::instance();

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

  Revision(const string& date, const string& message, const string& parents) {
    const auto& repo = Repository::instance();
    _parents = repo.extract_parents(parents);
    _message = message;
    _date = date;
  }

  string message() {
    return _message;
  }

  string date() {
    return _date;
  }

  optional<vector<string>> parents() {
    return _parents;
  }
};
