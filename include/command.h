#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Command {
  public:
  Command(const string command);
  Command& arg(const string arg);
  void printArgs();
  string invoke();

  private:
  vector<string> args;
  const string command;
};
