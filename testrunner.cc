#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ext/doctest.h"
#include "sqlwriter.hh"
using namespace std;

TEST_CASE("basic test") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"pief",1}});
  }
  MiniSQLite ms("testrunner-example.sqlite3");
  auto res=ms.exec("select pief from data");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="1");
  unlink("testrunner-example.sqlite3");
}


TEST_CASE("other table test") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"piefpaf", 2}}, "metadata");
  }
  MiniSQLite ms("testrunner-example.sqlite3");
  auto res=ms.exec("select piefpaf from metadata");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="2");
  unlink("testrunner-example.sqlite3");
}

TEST_CASE("test add columns") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"piefpaf", 2}});
    sqw.addValue({
        {"piefpaf", 3},
        {"poef", 1.02},
        {"poetput", "test"},
      });
    sqw.addValue({{"piefpaf", 1}});
  }
  MiniSQLite ms("testrunner-example.sqlite3");
  auto res=ms.exec("select sum(piefpaf), sum(poef) from data");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="6");
  CHECK((res[0][1])=="1.02");
  unlink("testrunner-example.sqlite3");
}

TEST_CASE("test multiple tables") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"piefpaf", 2}});
    sqw.addValue({{"piefpaf", 3}}, "metadata");
    sqw.addValue({{"poef", 21}});
    sqw.addValue({{"poef", 32}}, "metadata");    
  }
  MiniSQLite ms("testrunner-example.sqlite3");
  auto res=ms.exec("select sum(piefpaf), sum(poef) from data");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="2");
  CHECK((res[0][1])=="21");

  res=ms.exec("select sum(piefpaf), sum(poef) from metadata");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="3");
  CHECK((res[0][1])=="32");

  
  unlink("testrunner-example.sqlite3");
}


TEST_CASE("test scale") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    for(int n=0; n < 100000; ++n) {
      sqw.addValue({{"piefpaf", n}, {"wuh", 1.0*n}, {"wah", std::to_string(n)}});
      sqw.addValue({{"piefpaf", n}, {"wuh", 1.0*n}, {"wah", std::to_string(n)}}, "metadata");      
    }
    sqw.addValue({{"poef", 21}});
    sqw.addValue({{"poef", 32}}, "metadata");    
  }
  MiniSQLite ms("testrunner-example.sqlite3");
  auto res=ms.exec("select count(1) from data");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="100001");

  res=ms.exec("select count(1) from metadata");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="100001");

  res=ms.exec("select count(poef) from metadata");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="1");

  res=ms.exec("select count(wah) from metadata");
  CHECK(res.size() == 1);
  CHECK((res[0][0])=="100000");

  unlink("testrunner-example.sqlite3");
}
