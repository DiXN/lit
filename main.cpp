#include "command.h"
#include <iostream>

int main(void) {
  string cmd("diff");

  auto command = Command(cmd)
                  .arg("file1")
                  .arg("file2");

  command.printArgs();

  cout << "output for command \"diff\":\n" << command.invoke();
  return 0;
}
