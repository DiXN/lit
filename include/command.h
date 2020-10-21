#include <iostream>
#include <string>
#include <vector>
#include <tuple>

using namespace std;

class Command {
  public:
  Command(const string command);
  Command& arg(const string arg);
  void printArgs();
  tuple<string, int> invoke();

  private:
  vector<string> args;
  const string command;
};
