#include "ext/doctest.h"
#include "sqlwriter.hh"
#include "jsonhelper.hh"
#include <unistd.h>
using namespace std;

TEST_CASE("basic json test") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"pief",1}});
    sqw.addValue({{"paf",12.0}});
    sqw.addValue({{"user", "ahu"}, {"paf", 14}});
    sqw.addValue({{"user", "jhu"}, {"paf", 14.23}, {"pief", 99}});
  }
  // best way to guarantee that you can query the data you inserted is by closing the connection
  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res = sqw.queryT("select * from data");

  //  cout << packResultsJsonStr(res) << endl;
  
  nlohmann::json js = packResultsJson(res);
  CHECK(js[0]["pief"] == 1);
  CHECK(js[1]["paf"] == 12.0);
  CHECK(js[2]["user"] == "ahu");
  
  CHECK(res.size() == 4);

  res = sqw.queryT("select 1.0*sum(pief) as p from data");
  js = packResultsJson(res);
  CHECK(js[0]["p"]==100.0);
  CHECK(get<double>(res[0]["p"])==100.0);
  unlink("testrunner-example.sqlite3");
}


TEST_CASE("json helper")
{
  SQLiteWriter sqw("");
  sqw.addValue({{"id", 1}, {"user", "ahu"}});
  sqw.addValue({{"id", 2}});
  auto res = sqw.queryT("select * from data");
  REQUIRE(res.size() == 2);

  auto j = packResultsJson(res, false);
  REQUIRE(j.size() == 2);
  CHECK(j[0].count("user") == 1);
  CHECK(j[1].count("user") == 0);

  j = packResultsJson(res);
  REQUIRE(j.size() == 2);
  CHECK(j[0].count("user") == 1);
  CHECK(j[1].count("user") == 1);
  string user = j[1]["user"];
  CHECK(user =="");
}
