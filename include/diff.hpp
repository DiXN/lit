#pragma once

#include "repository.hpp"

#include <sstream>
#include <iostream>
#include <filesystem>
#include <ctime>

using namespace std;
namespace fs = std::filesystem;

class Diff {
  private:
  string _revision;

  public:
  Diff(const string& revision) : _revision(revision) {}
  Diff(const string&& revision) : _revision(revision) {}
  const Repository& repo = Repository::instance();

  void save(const bool has_previous_commit) const {
    ofstream revision_file(repo.get_lit_path() / "objects" / _revision);

    for (auto& p: fs::recursive_directory_iterator(repo.root_path())) {
      if (!repo.is_excluded(p) && !fs::is_directory(p)) {

        string comparer = "/dev/null";
        if (has_previous_commit) {
          const auto relative_path = fs::relative(p, repo.root_path());
          const auto previous_path = repo.get_lit_path() / "state" / "current" / relative_path;

          if (fs::exists(previous_path))
            comparer = previous_path.string();
        }

        auto command = Command(string("diff")).arg(string("-u")).arg(comparer).arg(p.path());

        const auto& [output, status_code] = command.invoke();
        revision_file << output;
      }
    }
  }

  enum DiffTypes {
    modified,
    deleted,
    added,
    none
  };

  static constexpr array<char, 4> diff_types_label {
    'M',
    'D',
    'A',
    '\0'
  };

  static map<fs::path, DiffTypes> file_differences(fs::path init_path, fs::path compare_path) {

    const Repository& repo = Repository::instance();
    vector<fs::path> deleted_paths;
    map<fs::path, DiffTypes> file_diffs;

    for (auto& p: fs::recursive_directory_iterator(compare_path)) {
      const auto& curr_relative = fs::relative(p, compare_path);
      const auto& search_path = init_path / curr_relative;

      if(!fs::exists(search_path)) {
        file_diffs.emplace(curr_relative, DiffTypes::deleted);
        deleted_paths.push_back(p);
      }
    }

    for (auto& p: fs::recursive_directory_iterator(init_path)) {
      if (!repo.is_excluded(p)) {
        const auto& curr_relative = fs::relative(p, init_path);
        const auto& search_path = compare_path / curr_relative;

        if(!fs::exists(search_path)) {
          file_diffs.emplace(curr_relative, DiffTypes::added);
        } else if (find(deleted_paths.begin(), deleted_paths.end(), p) == deleted_paths.end()) {
          auto command = Command("diff").arg(init_path / curr_relative).arg(search_path);
          const auto& [output, status] = command.invoke();

          if (status == 1)
            file_diffs.emplace(curr_relative.filename(), DiffTypes::modified);
        }
      }
    }

    return file_diffs;
  }
};
