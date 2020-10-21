#pragma once

#include "repository.hpp"
#include "arg.h"
#include "exec.hpp"
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <ctime>

using namespace std;
namespace fs = std::filesystem;

class Commit: public Arg {
  private:
  void write_commit(const Repository& repo, const stringstream& ss) const {
    ofstream commit;
    commit.open(repo.get_lit_path() / "branches" / repo.current_branch(), ofstream::out | ofstream::app);
    commit << ss.str() << endl;
    commit.close();
  }

  public:
  Commit() {}
  bool invoke(const optional<string> message) const override {
    const auto& repo = Repository::instance();

    const auto now = std::time(nullptr);
    const auto& last_commit = repo.last_commit();

    fs::path revision_path;
    if (!last_commit) {
      revision_path = repo.get_lit_path() / "objects" / "r1";

      stringstream commit;
      commit << "r1" << "|" << put_time(localtime(&now), "%c") << "|" << *message << endl;
      write_commit(repo, commit);

      repo.copy_structure(move("init"));
    } else {
      const auto& [last_commit_nr, time_of_commit, commit_message] = *last_commit;

      stringstream prev_revision(last_commit_nr);
      char prev_rev;
      int prev_rev_number;
      prev_revision >> prev_rev;
      prev_revision >> prev_rev_number;

      stringstream new_revision;
      new_revision << "r" << ++prev_rev_number << "|" << put_time(localtime(&now), "%c") << "|" << *message << endl;

      stringstream new_revision_file;
      new_revision_file << "r" << prev_rev_number;
      revision_path = repo.get_lit_path() / "objects" / new_revision_file.str();

      write_commit(repo, new_revision);

      repo.copy_structure(move("current"));
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

    return true;
  }


  ostringstream info() const override {
    ostringstream os;
    os << "Creates a commit." << endl;
    return os;
  }
};

