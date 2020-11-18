#pragma once

#include "arg.h"
#include "repository.hpp"

#include <filesystem>

namespace lit {

namespace fs = std::filesystem;

class Show : public Arg {
  public:
  Show() = default;
  bool invoke(const optional<string> arg) const override {
    const auto& repo = Repository::instance();
    if (!arg) {
      const auto& last_commit = repo.last_commit_of_branch(".last");

      if (!last_commit) {
        cerr << "Cant show last checked out commit, since there is no commit" << endl;
        return false;
      }

      const auto& [commit_nr, date, commit_message, parents] = *last_commit;

      cout << "Commit: " << commit_nr << endl;
      cout << "Parents: " << parents << endl;
      cout << "Date: " << date << endl << endl;
      cout << commit_message << endl << endl;

      ifstream patch_file(repo.get_lit_path() / "objects" / commit_nr);
      cout << patch_file.rdbuf();
      return true;
    }

    Revision revision(*arg);
    cout << "Commit: " << *arg << endl;
    cout << "Parents: ";
    const auto& rev_parents = revision.parents();

    if (!rev_parents) {
      cout << "root" << endl;
    } else {
      const auto& parents = *rev_parents;

      if (parents.size() > 1)
        cout << parents[0] << ", " << parents[1] << endl;
      else
        cout << parents[0] << endl;
    }

    cout << "Date: " << revision.date() << endl << endl;
    cout << revision.message() << endl << endl;

    ifstream patch_file(repo.get_lit_path() / "objects" / *arg);
    cout << patch_file.rdbuf();

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Shows information of the specified \"lit\" commit." << endl;
    return os;
  }
};

}; // namespace lit

