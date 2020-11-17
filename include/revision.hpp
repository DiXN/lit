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
};
