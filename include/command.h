#pragma once

#include <filesystem>
#include <tuple>
#include <vector>

namespace lit {

namespace fs = std::filesystem;

using namespace std;

class Command {
  public:
  Command(const string command);
  Command& arg(const string arg);
  Command& arg(const fs::path& arg);
  void printArgs();
  tuple<string, int> invoke();

  private:
  vector<string> args;
  const string command;
};

}; // namespace lit
