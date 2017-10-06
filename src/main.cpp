#include "application.cpp"

std::string VERSION = "0.6.1";

int main()
{
  Application app(VERSION);
  app.serve();
}
