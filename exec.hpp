#include <cstdio>
#include <cstring>
#include <istream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>

// ref: https://stackoverflow.com/a/63510594

using namespace std;

class execbuf: public streambuf {
  protected:
  string output;
  int_type underflow(int_type character) {
    if (gptr() < egptr())
      return traits_type::to_int_type(*gptr());

    return traits_type::eof();
  }

  public:
  execbuf(const char* command) {
    array<char, 128> buffer;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);

    if (!pipe) {
      throw runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      this->output += buffer.data();
    }

    char* begin_output = static_cast<char*>(this->output.data());
    char* end_output = static_cast<char*>(this->output.data() + this->output.size());
    setg(begin_output, begin_output, end_output);
  }
};

class exec: public istream {
  protected:
  execbuf buffer;

  public:
  exec(const char* command): istream(nullptr), buffer(command) {
    this->rdbuf(&buffer);
  };
  ostringstream output() {
    std::ostringstream os;
    os << this->rdbuf();
    return os;
  };
};
