#pragma once

#include "repository.hpp"
#include "arg.h"
#include "revision.hpp"

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
  bool invoke(const optional<string> message) const override {
    const auto& repo = Repository::instance();

    const auto& last_commit = repo.last_commit_of_branch(".last");

    fs::path revision_path;
    if (!last_commit) {
      revision_path = repo.get_lit_path() / "objects" / "r0";

      Revision revision(*message, "root", 0);
      revision.write(repo.current_branch());

      repo.copy_structure(move("init"));
      repo.copy_structure(move("current"));
    } else {
      const auto& [last_commit_nr, date, commit_message, parents] = *last_commit;
      int commit_id = repo.unique_commit_id();
      const auto& current_branch = repo.current_branch();

      Revision revision(*message, last_commit_nr, commit_id);

      stringstream new_revision_file;
      new_revision_file << "r" << commit_id;
      revision_path = repo.get_lit_path() / "objects" / new_revision_file.str();

      if (!repo.is_head(last_commit_nr)) {
        repo.create_branch(last_commit_nr);
        repo.copy_commit_structure(last_commit_nr, current_branch);
        revision.write(last_commit_nr);
        repo.switch_branch(last_commit_nr);
      } else {
        revision.write(current_branch);
      }
    }

    ofstream revision;
    revision.open(revision_path.string(), ofstream::out | ofstream::app);

    for (auto& p: fs::recursive_directory_iterator(repo.root_path())) {
      if (!repo.is_excluded(p) && !fs::is_directory(p)) {

        string comparer = "/dev/null";
        if (last_commit) {
          const auto relative_path = fs::relative(p, repo.root_path());
          const auto previous_path = repo.get_lit_path() / "state" / "current" / relative_path;

          if (fs::exists(previous_path))
            comparer = previous_path.string();
        }

        auto command = Command(string("diff")).arg(string("-u")).arg(comparer).arg(p.path());

        const auto& [output, status_code] = command.invoke();
        revision << output;
      }
    }

    revision.close();
    repo.copy_structure(move("current"));

    return true;
  }


  ostringstream info() const override {
    ostringstream os;
    os << "Creates a commit." << endl;
    return os;
  }
};

