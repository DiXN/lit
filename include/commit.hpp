#pragma once

#include "repository.hpp"
#include "arg.h"
#include "exec.hpp"
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>

using namespace std;
namespace fs = std::filesystem;

class Commit: public Arg {
  public:
  Commit() {}
  void invoke() const override {
    const auto& repo = Repository::instance();

    const auto& last_commit = repo.last_commit();

    fs::path revision_path;
    if (last_commit == nullopt) {
      revision_path = repo.get_lit_path() / "objects" / "r1";
      ofstream commit;
      commit.open(repo.get_lit_path() / "branches" / repo.current_branch(), ofstream::out | ofstream::app);
      commit << "r1" << endl;
      commit.close();
    } else {
      stringstream prev_revision(*last_commit);
      char prev_rev;
      int prev_rev_number;
      prev_revision >> prev_rev;
      prev_revision >> prev_rev_number;

      stringstream new_revision;
      new_revision << "r" << ++prev_rev_number;
      revision_path = repo.get_lit_path() / "objects" / new_revision.str();

      ofstream commit;
      commit.open(repo.get_lit_path() / "branches" / repo.current_branch(), ofstream::out | ofstream::app);
      commit << new_revision.str() << endl;
      commit.close();
    }

    ofstream revision;
    revision.open(revision_path.string(), ofstream::out | ofstream::app);

    for (auto& p: fs::recursive_directory_iterator(repo.root_path())) {
      if (!repo.is_excluded(p)) {
        string cmd("diff");
        auto command = Command(cmd).arg("-u").arg("/dev/null").arg(p.path().string());
        revision << command.invoke();
      }
    }

    revision.close();
  }


  ostringstream info() const override {
    ostringstream os;
    os << "Creates a commit." << endl;
    return os;
  }
};

