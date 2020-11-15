#include "command.h"
#include "arg.h"
#include "init.hpp"
#include "commit.hpp"
#include "status.hpp"
#include "checkout.hpp"
#include "merge.hpp"
#include "log.hpp"
#include "show.hpp"

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <numeric>

map<string, unique_ptr<Arg>> init_arg_register() {
  map<string, unique_ptr<Arg>> args;
  args.emplace("init", make_unique<Init>());
  args.emplace("commit", make_unique<Commit>());
  args.emplace("status", make_unique<Status>());
  args.emplace("checkout", make_unique<Checkout>());
  args.emplace("merge", make_unique<Merge>());
  args.emplace("log", make_unique<Log>());
  args.emplace("show", make_unique<Show>());
  return args;
}

void print_help(map<string, unique_ptr<Arg>>& args) {
  cout << "available commands in lit:" << endl;

  for (const auto& [name, arg]: args) {
    cout << " " << name << ": " << arg->info().str();
  }
}

bool match_arg(const string& arg, map<string, unique_ptr<Arg>>& args, string cmd_args) {
  const auto match = args.find(arg);

  if (match == args.end()) {
    cerr << "lit: \"" << arg << "\" is not a lit command." << endl << endl;
    print_help(args);
    return false;
  }

  optional<string> optional_cmd_args = nullopt;

  if (!cmd_args.empty()) {
    optional_cmd_args = optional<string>(cmd_args);
  }

  return args[arg]->invoke(move(optional_cmd_args));
}


int main(int argc, char** argv) {
  auto args = init_arg_register();

  if (argc == 1) {
    print_help(args);
    return 0;
  }

  const auto& cmd_args =
      std::accumulate(argv + 2, argv + argc, string(""),
          [](const string& a, string b) { return a + (!a.empty() ? " " : "") + b; });

  if (!match_arg(argv[1], args, move(cmd_args))) {
    return 1;
  }

  return 0;
}

