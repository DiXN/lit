#include "command.h"
#include <iostream>
#include <sstream>
#include <ostream>
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

tuple<string, int> Command::invoke() {
  const auto& cmd =
      std::accumulate(args.begin(), args.end(), command, [](const string& a, string b) { return a + " " + b; });

  ostringstream oss;
  oss << "sh -c " << "\"" << cmd << "\"" << " 2>&1";

  array<char, 128> buffer;

  auto pipe = popen(oss.str().c_str(), "r");

  if (!pipe) {
    throw runtime_error("popen failure");
  }

  stringstream output;

  while (fgets(buffer.data(), buffer.size(), pipe)) {
    output << buffer.data();
  }

  auto exit_code = pclose(pipe);

  if(exit_code == -1) {
    throw runtime_error("pclose failure");
  }

  return make_tuple(output.str(), exit_code);
}

