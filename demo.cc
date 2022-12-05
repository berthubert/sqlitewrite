#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include "sqlwriter.hh"

using namespace std;


int main()
try
{
  SQLiteWriter sqw("example.sqlite3");

  for(int n = 0 ; n < 1000000; ++n) {
    sqw.addValue({{"pief", n}, {"poef", 1.1234567890123*n}, {"paf", "bert"}});
  }

  sqw.addValue({{"timestamp", 1234567890}});

  for(int n = 0 ; n < 1; ++n) {
    sqw.addValue({{"pief", n}, {"poef", 1.1234567890123*n}, {"paf", "bert"}});
  }

}
catch (std::exception& e)
{
    std::cout << "exception: " << e.what() << std::endl;
}
