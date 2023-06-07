#include "psqlwriter.hh"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
try
{
  PSQLWriter sqw("");
  for(unsigned n=0; n < 50000; ++n) {
    sqw.addValue({{"test", 1},
                  {"value", 2},
                  {"name", "bert"},
                  {"temp", 12.0}, {"n", n}, {"cool", (n%2) ? false : true}});
  }
  for(unsigned n=0; n < 5000; ++n) {
    sqw.addValue({{"joh", 1.1*n}, {"wuh", to_string(n)+"_uhuh"}}, "vals");
  }
  for(unsigned n=0; n < 50000; ++n) {
    sqw.addValue({{"pief", 1},
                  {"poef", 2},
                  {"paf", "bert"},
                  {"temp", 12.0}, {"n", n}, {"cool", (n%2) ? false : true}});
  }

  cout<<"Done with adding"<<endl;

}
catch (const std::exception &e)
{
  std::cerr << e.what() << std::endl;
  return 1;
}
