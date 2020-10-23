#pragma once

#include "repository.hpp"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

class Status: public Arg {
  public:
  Status() {}
  bool invoke(const optional<string> arg) const override {
    if (arg) {
      cerr << "On command 'lit status' no argument is allowed" << endl;
      return false;
    }

    const auto& repo = Repository::instance();
    const auto& last_commit = repo.last_commit_of_branch(".total");

    if (!last_commit) {
      for (auto& p: fs::recursive_directory_iterator(repo.root_path())) {
        if (!repo.is_excluded(p)) {
          cout << "A  " << fs::relative(p, repo.root_path()) << endl;
        }
      }
    } else {
      vector<fs::path> deleted_paths;

      const auto& previous_repo_state_path = repo.get_lit_path() / "state" / "current";
      for (auto& p: fs::recursive_directory_iterator(previous_repo_state_path)) {
        const auto& curr_relative = fs::relative(p, previous_repo_state_path);
        const auto& search_path = repo.root_path() / curr_relative;

        if(!fs::exists(search_path)) {
          cout << "D  " << curr_relative << endl;
          deleted_paths.push_back(p);
        }
      }

      for (auto& p: fs::recursive_directory_iterator(repo.root_path())) {
        if (!repo.is_excluded(p)) {
          const auto& curr_relative = fs::relative(p, repo.root_path());
          const auto& search_path = repo.get_lit_path() / "state" / "current" / curr_relative;

          if(!fs::exists(search_path)) {
            cout << "A  " << curr_relative << endl;
          } else if (find(deleted_paths.begin(), deleted_paths.end(), p) == deleted_paths.end()) {
            auto command = Command("diff").arg(curr_relative).arg(search_path);
            const auto& [output, status] = command.invoke();

            if (status == 1) {
              cout << "M  " << curr_relative << endl;
            }
          }
        }
      }

    }

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Shows the status of the \"lit\" repository." << endl;
    return os;
  }
};

