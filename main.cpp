#include "command.h"
#include "arg.h"
#include "init.hpp"
#include <iostream>
#include <memory>
#include <map>
#include <string>

map<string, unique_ptr<Arg>> init_arg_register() {
  map<string, unique_ptr<Arg>> args;
  args.emplace("init", make_unique<Init>());
  return args;
}

bool match_arg(const string& arg, map<string, unique_ptr<Arg>>& args) {
  const auto match = args.find(arg);

  if (match == args.end()) {
    return false;
  }

  return true;
}

void print_help(map<string, unique_ptr<Arg>>& args) {
  cout << "available commands in lit:" << endl;

  for (const auto& [name, arg]: args) {
    cout << " " << name << ": " << arg->info().str() << endl;
  }
}

int main(int argc, char** argv) {
  auto args = init_arg_register();

  if (!match_arg(argv[1], args)) {
    cerr << "lit: \"" << argv[1] << "\" is not a lit command." << endl << endl;
    print_help(args);
    return 1;
  }

  string cmd("diff");

  auto command = Command(cmd).arg("file1").arg("file2");

  command.printArgs();

  cout << "output for command \"diff\":\n" << command.invoke();
  return 0;
}

