#pragma once

#include "repository.hpp"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <filesystem>
#include <tuple>

using namespace std;
namespace fs = std::filesystem;

class Log: public Arg {
  public:
  Log() {}
  bool invoke(const optional<string> arg) const override {
    if (arg) {
      cerr << "On command 'lit Log' no argument is allowed" << endl;
      return false;
    }

    const auto& repo = Repository::instance();

    ifstream branch_file(repo.get_lit_path() / "branches" / ".total");

    string commit;
    string time;
    string message;
    string line;

    vector<string> reverse_lines;
    map<string, tuple<int, int>> branches_to_read_from;

    while(getline(branch_file, line)) {
      reverse_lines.push_back(line);
    }

    int horizontal_offset = 0;
    int commit_offset = 0;
    for (auto& l : reverse_lines) {
      array<string, 3> tokens = repo.extract_commit_information(l, '|');
      commit = tokens[0];

      branches_to_read_from.emplace("master", make_tuple(horizontal_offset, commit_offset));

      for (auto& b: fs::directory_iterator(repo.get_lit_path() / "branches")) {
        const auto& file_name = b.path().filename();

        if(file_name == commit) {
          branches_to_read_from.emplace(file_name, make_tuple(++horizontal_offset, commit_offset));
        }
      }

      commit_offset++;
    }

    vector<string> art(reverse_lines.size(), "");
    vector<int> commit_indices;

    for (const auto &[branch, offset]: branches_to_read_from) {
      auto &[h_offset, c_offset] = offset;

      ifstream branch_file(repo.get_lit_path() / "branches" / branch);

      int last_index = 0;
      while(getline(branch_file, line)) {
        array<string, 3> branch_tokens = repo.extract_commit_information(line, '|');
        auto& branch_commit = branch_tokens[0];

        int index = std::stoi(branch_commit.erase(0, 1));
        commit_indices.push_back(index);

        if (branch != "master" &&  c_offset == index) {
          art[index] += "--◯";
        } else {
          if(branch != "master") {
            stringstream ss;
            ss << setw(h_offset * 6) << "◯";
            art[index] += ss.str();
          } else {
            art[index] += "◯";
          }
        }

        last_index = index;
      }

      for(size_t i = 0; i < reverse_lines.size(); i++) {
        if(std::find(commit_indices.begin(), commit_indices.end(), i) == commit_indices.end() && i <= last_index) {
          stringstream ss;
          ss << setw(h_offset * 3) << "|";
          art[i] += ss.str();
        }
      }

      commit_indices.clear();
    }

    commit_offset = art.size() - 1;

    for(auto it = reverse_lines.rbegin(); it != reverse_lines.rend(); ++it) {
      array<string, 3> tokens = repo.extract_commit_information(*it, '|');
      commit = tokens[0];
      time = tokens[1];
      message = tokens[2];

      cout << art[commit_offset] << "\t" << commit << " " << message << endl;
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

