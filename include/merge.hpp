#pragma once

#include "repository.hpp"
#include "arg.h"
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
        const auto& commit_id = *arg;
        repo.checkout_commit(commit_id, *branch, lit_merge_path, false);

        const auto& lit_merge_parent_path = lit_merge_path.parent_path();
        const auto& comparer_path = lit_merge_parent_path / "comparer";

        repo.checkout_commit(*branch, *branch, comparer_path, false);

        for(const auto &[path, diff_type] : repo.file_differences(comparer_path, lit_merge_path)) {
          cout << repo.diff_types_label[diff_type] << "  " << path << endl;
          switch (diff_type) {
            case Repository::DiffTypes::added:
              fs::copy(lit_merge_path / path, repo.root_path() / path);
              break;
            case Repository::DiffTypes::deleted:
              fs::remove_all(repo.root_path() / path);
              break;
            case Repository::DiffTypes::modified:
              const auto& lit_temp_path = lit_merge_path.parent_path();
              const auto& file_name = path.filename();

              const auto& curr_relative = comparer_path / file_name;
              const auto& search_path = repo.root_path() / file_name;

              auto command = Command("diff").arg(curr_relative).arg(search_path);
              const auto& [output, status] = command.invoke();

              if (status == 1) {
                potential_conflict = true;
                cout << "conflict potential!" << endl;
                fs::copy(lit_merge_path / path, path.string() + "." + *arg);
                fs::copy(comparer_path / path, path.string() + "." + *branch);
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

          stringstream commit_stream;
          commit_stream << "Merge " << *arg << " into " << current_branch;

          int commit_id = repo.unique_commit_id();
          stringstream new_revision;
          const auto now = std::time(nullptr);

          new_revision << "r" << commit_id << "|" << put_time(localtime(&now), "%c") << "|" << commit_stream.str() << endl;
          repo.write_commit(current_branch, new_revision);
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

