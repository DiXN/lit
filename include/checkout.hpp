#pragma once

#include "repository.hpp"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <filesystem>
#include <optional>

using namespace std;
namespace fs = std::filesystem;

class Checkout: public Arg {
  public:
  Checkout() {}
  bool invoke(const optional<string> arg) const override {
    const auto& repo = Repository::instance();

    if (!arg) {
      repo.clean();

      const auto copy_options = fs::copy_options::overwrite_existing
                              | fs::copy_options::recursive;

      const auto previous_files = repo.get_lit_path() / "state" / "current";

      for (auto& p: fs::directory_iterator(previous_files)) {
        fs::copy(p, repo.root_path(), copy_options);
      }

      return true;
    } else {
      if (repo.checkout_commit(*arg))
        return true;
    }

    cerr << "commit could not be found." << endl;
    return false;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Switches branch to the specified commit." << endl;
    return os;
  }
};

