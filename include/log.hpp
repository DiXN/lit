#pragma once

#include "arg.h"
#include "repository.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <tuple>

using namespace std;
namespace fs = std::filesystem;

class Log : public Arg {
  public:
  Log() {}
  bool invoke(const optional<string> arg) const override {
    if (arg) {
      cerr << "On command 'lit Log' no argument is allowed" << endl;
      return false;
    }

    const auto& repo = Repository::instance();

    const auto merge_check = [&repo](const string& revision_id) -> optional<tuple<string, string>> {
      Revision revision(revision_id);
      const auto& parents = revision.parents();

      if (!parents)
        return nullopt;

      if ((*parents).size() == 2) {
        const auto branch_a = *revision.branch();

        Revision parent_revision((*parents)[1]);
        const auto branch_b = *parent_revision.branch();

        return optional(make_tuple(branch_a, branch_b));
      }

      return nullopt;
    };

    ifstream branch_file(repo.get_lit_path() / "branches" / ".total");

    string commit;
    string time;
    string message;
    string line;

    vector<string> reverse_lines;
    map<string, tuple<size_t, size_t>> branches_to_read_from;

    while (getline(branch_file, line)) {
      reverse_lines.push_back(line);
    }

    int horizontal_offset = 0;
    size_t commit_offset = 0;
    for (auto& l: reverse_lines) {
      array<string, 4> tokens = repo.extract_commit_information(l, '|');
      commit = tokens[0];

      branches_to_read_from.emplace("master", make_tuple(horizontal_offset, commit_offset));

      for (auto& b: fs::directory_iterator(repo.get_lit_path() / "branches")) {
        const auto& file_name = b.path().filename();

        if (file_name == commit) {
          branches_to_read_from.emplace(file_name, make_tuple(++horizontal_offset, commit_offset));
        }
      }

      commit_offset++;
    }

    vector<string> art(reverse_lines.size(), "");
    vector<size_t> commit_indices;

    for (const auto& [branch, offset]: branches_to_read_from) {
      auto& [h_offset, c_offset] = offset;

      ifstream branch_file(repo.get_lit_path() / "branches" / branch);

      size_t last_index = 0;
      while (getline(branch_file, line)) {
        array<string, 4> branch_tokens = repo.extract_commit_information(line, '|');
        auto& branch_commit = branch_tokens[0];

        size_t index = std::stoi(branch_commit.erase(0, 1));
        commit_indices.push_back(index);

        if (index >= c_offset) {
          if (branch != "master" && index == c_offset) {
            art[index] += "--◯";
          } else {
            if (branch != "master") {
              stringstream ss;

              if (!art[index].empty())
                ss << setw(h_offset > 1 ? h_offset * 6 - 4 : h_offset * 5) << "◯";
              else
                ss << setw(h_offset > 1 ? h_offset * 6 - 3 : h_offset * 6) << "◯";

              art[index] += ss.str();
            } else {
              art[index] += "◯";
            }
          }
        }

        const auto& merge_line = merge_check("r" + branch_commit);

        if (merge_line) {
          const auto& [branch_a, branch_b] = *merge_line;
          if (branch_a == branch) {
            const auto& [other_h_offset, other_c_offset] = branches_to_read_from[branch_b];
            const auto distance = other_h_offset - h_offset;

            for (size_t i = 0; i < distance * 2 + (other_h_offset - 1); i++) {
              art[index] += "-";
            }
          }
        }

        last_index = index;
      }

      for (size_t i = 0; i < reverse_lines.size(); i++) {
        if (std::find(commit_indices.begin(), commit_indices.end(), i) == commit_indices.end() && i <= last_index &&
            i >= c_offset) {
          if (art[i][art[i].size() - 1] == '-') {
            art[i] += "|";
          } else {
            stringstream ss;
            ss << setw(h_offset > 1 ? 3 : h_offset * 3) << "|";
            art[i] += ss.str();
          }
        }

        auto search_index = last_index + 1;

        if (reverse_lines.size() > search_index && i == search_index) {
          const auto& merge_line = merge_check("r" + to_string(search_index));

          if (merge_line) {
            if (art[i][art[i].size() - 1] == '-') {
              art[i] += "¬";
            } else {
              stringstream ss;
              ss << setw(h_offset * 3) << "ı";
              art[i] += ss.str();
            }
          }
        }
      }

      commit_indices.clear();
    }

    commit_offset = art.size() - 1;

    for (auto it = reverse_lines.rbegin(); it != reverse_lines.rend(); ++it) {
      array<string, 4> tokens = repo.extract_commit_information(*it, '|');
      commit = tokens[0];
      time = tokens[1];
      message = tokens[2];

      ifstream checked_out_commit(repo.get_lit_path() / "branches" / ".last");
      string line;
      getline(checked_out_commit, line);
      array<string, 4> branch_tokens = repo.extract_commit_information(line, '|');
      auto& branch_commit = branch_tokens[0];
      size_t checked_out_index = std::stoi(branch_commit.erase(0, 1));

      if (commit_offset != checked_out_index)
        cout << art[commit_offset] << "\t"
             << "  " << commit << " " << message << endl;
      else
        cout << art[commit_offset] << "\t"
             << "← " << commit << " " << message << endl;

      commit_offset--;
    }

    return true;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Prints the graph for the \"lit\" repository." << endl;
    return os;
  }
};
