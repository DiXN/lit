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
        if(p.path().filename() == ".last") {
          continue;
        }

        ifstream branch_file(p.path());

        string commit;
        string line;

        while(getline(branch_file, line)) {
          array<string, 3> tokens = repo.extract_commit_information(line, '|');
          commit = tokens[0];

          if (commit == commit_id) {
            repo.switch_branch(p.path().filename());
            ofstream last(repo.get_lit_path() / "branches" / ".last");
            last << line;

            repo.clean();
            for (auto& patch_files: fs::directory_iterator(repo.get_lit_path() / "objects")) {
              auto command = Command("patch").arg(string("-s")).arg(string("-i")).arg(patch_files.path());

              const auto& [output, status] = command.invoke();
              if (patch_files.path().filename() == commit) {
                break;
              }
            }
            return true;
          }
        }

        cerr << "commit could not be found." << endl;
        return false;
      }
    }

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Switches branch to the specified commit." << endl;
    return os;
  }
};

