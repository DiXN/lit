#pragma once

#include "repository.hpp"
#include "arg.h"
#include "revision.hpp"
#include "diff.hpp"

#include <sstream>
#include <iostream>
#include <filesystem>
#include <tuple>
#include <ctime>

using namespace std;
namespace fs = std::filesystem;

class Merge: public Arg {
  private:
  tuple<bool, fs::path> prepare_merge() const {
    const auto& temp_path = fs::temp_directory_path();
    const auto& lit_merge_path = temp_path / "lit" / "merge";
    const auto& comparer = temp_path / "lit" / "comparer";

    fs::remove_all(comparer);
    fs::remove_all(lit_merge_path);
    const auto comparer_success = fs::create_directories(comparer);
    const auto merge_success = fs::create_directories(lit_merge_path);
    return make_tuple(comparer_success && merge_success, lit_merge_path);
  }

  public:
  Merge() {}
  bool invoke(const optional<string> arg) const override {
    const auto& repo = Repository::instance();

    if (!arg) {
      cerr << "On command 'lit merge' an argument is required" << endl;
      return false;
    } else {
      const auto& [temp_path_success, lit_merge_path] = prepare_merge();

      if(!temp_path_success) {
        cerr << "Could not create temporary directory, that is necessary for merge" << endl;
        return false;
      }


      const auto& branch = repo.find_branch_for_commit(*arg);
      bool potential_conflict = false;

      if(branch) {
        const auto& base_of_branches = repo.find_base_commit(repo.current_branch(), *branch);

        if(!base_of_branches) {
          cerr << "Cannot find common base of branch: \"" << repo.current_branch() << "\" and branch: \"" << *branch << "\"" << endl;
          return false;
        }

        const auto& commit_id = *arg;
        repo.checkout_commit(commit_id, *branch, lit_merge_path, false);

        const auto& lit_merge_parent_path = lit_merge_path.parent_path();
        const auto& comparer_path = lit_merge_parent_path / "comparer";

        repo.checkout_commit(*base_of_branches, *branch, comparer_path, false);

        for(const auto &[path, diff_type] : Diff::file_differences(lit_merge_path, comparer_path)) {
          cout << Diff::diff_types_label[diff_type] << "  " << path << endl;
          switch (diff_type) {
            case Diff::DiffTypes::added:
              if (fs::is_directory(repo.root_path() / path)) {
                fs::create_directories(lit_merge_path / path);
              } else {
                fs::copy(lit_merge_path / path, repo.root_path() / path, fs::copy_options::overwrite_existing|fs::copy_options::recursive);
              }
              break;
            case Diff::DiffTypes::deleted:
              fs::remove_all(repo.root_path() / path);
              break;
            case Diff::DiffTypes::modified:
              const auto& lit_temp_path = lit_merge_path.parent_path();

              const auto& curr_relative = fs::relative(path, repo.root_path());
              const auto& search_path = comparer_path / path;

              auto command = Command("diff").arg(curr_relative).arg(search_path);
              const auto& [output, status] = command.invoke();

              if (status == 1) {
                potential_conflict = true;
                fs::copy(lit_merge_path / path, path.string() + "." + *arg);
                fs::copy(comparer_path / path, path.string() + "." + *base_of_branches);
              } else {
                const auto copy_options = fs::copy_options::overwrite_existing
                                           | fs::copy_options::recursive;

                fs::copy(lit_merge_path / path, repo.root_path() / path, copy_options);
              }

              break;
          }
        }

        if(!potential_conflict) {
          const auto& current_branch = repo.current_branch();
          const auto &[current_commit_nr, current_date, current_message, parents] = *repo.last_commit_of_branch(current_branch);

          stringstream commit_stream;
          commit_stream << "Merge " << *arg << " into " << current_branch;

          Revision revision(commit_stream.str(), current_commit_nr + "," + *arg);
          revision.write(current_branch);
          const Diff diff(revision.revision());
          diff.save(true);
        } else {
          ofstream merge_progress(repo.get_lit_path() / "merge");
          merge_progress << *arg;
        }
      }
    }

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Merges two \"lit\" commits." << endl;
    return os;
  }
};

