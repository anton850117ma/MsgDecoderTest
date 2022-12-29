// main.cpp
#include <iostream>
extern constinit int staticA;
auto       staticB = staticA;
int        main() { std::cout << "staticB: " << staticB << std::endl; }