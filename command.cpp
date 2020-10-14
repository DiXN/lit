#include "command.h"
#include "exec.hpp"
#include <iostream>
#include <numeric>

Command::Command(const string command): command(std::move(command)) {}

Command& Command::arg(const string arg) {
  args.push_back(std::move(arg));
  return *this;
}

void Command::printArgs() {
  std::cout << "arguments for command \"" << command << "\":" << std::endl;

  for (const auto& arg: args) {
    std::cout << arg << "\n";
  }
}

string Command::invoke() {
  const auto& cmd =
      std::accumulate(args.begin(), args.end(), command, [](const string& a, string b) { return a + " " + b; });

  ostringstream oss;
  oss << "sh -c " << cmd << " 2>&1";

  auto output = exec(std::move(oss.str().c_str())).output();

  return std::move(output.str());
}
