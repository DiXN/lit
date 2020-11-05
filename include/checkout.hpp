#pragma once

#include "repository.hpp"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

class Checkout: public Arg {
  public:
  Checkout() {}
  bool invoke(const optional<string> arg) const override {
    const auto& repo = Repository::instance();

    if (!arg) {
      const auto copy_options = fs::copy_options::overwrite_existing
                              | fs::copy_options::recursive;

      const auto previous_files = repo.get_lit_path() / "state" / "current";

      for (auto& p: fs::directory_iterator(previous_files)) {
        fs::copy(p, repo.root_path(), copy_options);
      }
    } else {
      const auto& commit_id = *arg;

      for (auto& p: fs::directory_iterator(repo.get_lit_path() / "branches")) {
        if(p.path().filename() == ".last" || p.path().filename() == ".total") {
          continue;
        }

        cout << "looking for commit in " << p.path().filename() << endl;

        ifstream branch_file(p.path());

        string commit;
        string line;
        vector<string> commits_in_branch;

        while(getline(branch_file, line)) {
          array<string, 3> tokens = repo.extract_commit_information(line, '|');
          commit = tokens[0];
          commits_in_branch.push_back(commit);

          if (commit == commit_id) {
            repo.switch_branch(p.path().filename());
            ofstream last(repo.get_lit_path() / "branches" / ".last");
            last << line;

            repo.clean();

            for (auto& com : commits_in_branch) {
              const auto& patch_file = repo.get_lit_path() / "objects" / com;
              cout << patch_file << endl;
              auto command = Command("patch").arg(string("-s")).arg(string("-i")).arg(patch_file);
              const auto& [output, status] = command.invoke();
            }

            repo.copy_structure(move("current"));
            return true;
          }
        }

        commits_in_branch.clear();
      }
    }

    cerr << "commit could not be found." << endl;
    return false;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Switches branch to the specified commit." << endl;
    return os;
  }
};

