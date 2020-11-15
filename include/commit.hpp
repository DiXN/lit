#pragma once

#include "repository.hpp"
#include "arg.h"
#include "revision.hpp"

#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <ctime>

using namespace std;
namespace fs = std::filesystem;

class Commit: public Arg {
  public:
  Commit() {}
  bool invoke(const optional<string> message) const override {
    const auto& repo = Repository::instance();

    const auto now = std::time(nullptr);
    const auto& last_commit = repo.last_commit_of_branch(".last");

    fs::path revision_path;
    if (!last_commit) {
      revision_path = repo.get_lit_path() / "objects" / "r0";

      stringstream commit;
      commit << "r0" << "|" << put_time(localtime(&now), "%c") << "|" << *message << "|" << "root" << endl;
      repo.write_commit(repo.current_branch(), commit);

      repo.copy_structure(move("init"));
      repo.copy_structure(move("current"));
    } else {
      const auto& [last_commit_nr, date, commit_message] = *last_commit;
      int commit_id = repo.unique_commit_id();

      stringstream new_revision;
      new_revision << "r" << commit_id << "|" << put_time(localtime(&now), "%c") << "|" << *message << "|" << last_commit_nr << endl;

      stringstream new_revision_file;
      new_revision_file << "r" << commit_id;
      revision_path = repo.get_lit_path() / "objects" / new_revision_file.str();

      if (!repo.is_head(last_commit_nr)) {
        repo.create_branch(last_commit_nr);
        repo.copy_commit_structure(last_commit_nr, repo.current_branch());
        repo.write_commit(last_commit_nr, new_revision);
        repo.switch_branch(last_commit_nr);
      } else {
        repo.write_commit(repo.current_branch(), new_revision);
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

