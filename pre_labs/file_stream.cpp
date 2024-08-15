#include <fstream>
#include <iostream>

int main() {
  std::ifstream input;

  input.open("pg-dorian_gray.txt");
  std::string line;

  while (getline(input, line)) {
    std::cout << line << std::endl;
  }
  input.close();
  return 0;
}
