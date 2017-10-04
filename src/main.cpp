#include "application.cpp"

std::string VERSION = "0.5.3";

int main()
{
  Application app(VERSION);
  app.serve();
}
