#pragma once

#include "arg.h"
#include "repository.hpp"
#include "revision.hpp"

#include <filesystem>

namespace lit {

namespace fs = std::filesystem;

class Checkout : public Arg {
  public:
  Checkout() = default;
  bool invoke(const optional<string> arg) const override {
    const auto& repo = Repository::instance();

    if (!arg) {
      repo.clean();

      const auto copy_options = fs::copy_options::overwrite_existing | fs::copy_options::recursive;

      const auto previous_files = repo.get_lit_path() / "state" / "current";

      for (auto& p: fs::directory_iterator(previous_files)) {
        fs::copy(p, repo.root_path(), copy_options);
      }

      return true;
    } else {
      if (Revision::checkout_revision(*arg))
        return true;
    }

    cerr << "Revision could not be found." << endl;
    return false;
  }

  ostringstream info() const override {
    ostringstream os;
    os << "Switches to the specified \"lit\" revision." << endl;
    return os;
  }
};

}; // namespace lit

