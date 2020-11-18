#pragma once

#include "arg.h"
#include "diff.hpp"
#include "repository.hpp"
#include "revision.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

namespace lit {

using namespace std;
namespace fs = std::filesystem;

class Commit : public Arg {
  public:
  Commit() {}
  bool invoke(const optional<string> message) const override {
    const auto& repo = Repository::instance();

    const auto& last_commit = repo.last_commit_of_branch(".last");

    string revision_id;
    if (!last_commit) {
      revision_id = "r0";

      Revision revision(*message, "root", 0);
      revision.write(repo.current_branch());

      repo.copy_structure(move("current"));
    } else {
      const auto& [last_commit_nr, date, commit_message, parents] = *last_commit;
      const auto& current_branch = repo.current_branch();

      const auto& merge_progress = repo.get_lit_path() / "merge";
      stringstream ss;
      if (fs::exists(merge_progress)) {
        ifstream m(merge_progress);
        ss << "," << m.rdbuf();
        fs::remove(merge_progress);
      }

      Revision revision(*message, last_commit_nr + ss.str());

      if (!repo.is_head(last_commit_nr)) {
        repo.create_branch(last_commit_nr);
        repo.copy_commit_structure(last_commit_nr, current_branch);
        revision.write(last_commit_nr);
        repo.switch_branch(last_commit_nr);
      } else {
        revision.write(current_branch);
      }

      revision_id = revision.revision();
    }

    Diff diff(revision_id);

    if (last_commit)
      diff.save(true);
    else
      diff.save(false);

    repo.copy_structure(move("current"));

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Creates a \"lit\" commit.." << endl;
    return os;
  }
};

}; // namespace lit
