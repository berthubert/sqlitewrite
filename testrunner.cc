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

TEST_CASE("test queries") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"piefpaf", 2}});
    sqw.addValue({{"piefpaf", 7}});
    sqw.addValue({{"piefpaf", 3}}, "metadata");
    sqw.addValue({{"poef", 21}});
    sqw.addValue({{"poef", 32}}, "metadata");
    sqw.addValue({{"poef", -1}}, "metadata");    
  }

  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res = sqw.query("select * from data where piefpaf = ?", {2});
  CHECK(res.size() == 1);
  CHECK(res[0]["piefpaf"]=="2");

  res = sqw.query("select sum(piefpaf) as s from data");
  CHECK(res.size() == 1);
  CHECK(res[0]["s"]=="9");

  res = sqw.query("select piefpaf from data where piefpaf NOT NULL order by piefpaf");
  CHECK(res.size() == 2);
  CHECK(res[0]["piefpaf"]=="2");
  CHECK(res[1]["piefpaf"]=="7");

  res = sqw.query("select sum(poef) as s from metadata where poef > ?", {-2});
  CHECK(res.size() == 1);
  CHECK(res[0]["s"]=="31");

  res = sqw.query("select sum(poef) as s from metadata where poef > ?", {-1});
  CHECK(res.size() == 1);
  CHECK(res[0]["s"]=="32");
  
  unlink("testrunner-example.sqlite3");
}

TEST_CASE("test queries typed") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"piefpaf", 2}});
    sqw.addValue({{"piefpaf", 7}});
    sqw.addValue({{"poef", 21}});
    sqw.addValue({{"user", "ahu"}, {"piefpaf", 42}});
    sqw.addValue({{"temperature", 18.2}, {"piefpaf", 3}});

    sqw.addValue({{"poef", 8}}, "metadata");
    sqw.addValue({{"poef", 23}}, "metadata");
    sqw.addValue({{"poef", -1}}, "metadata");
    sqw.addValue({{"poef", -2}}, "metadata");
  }

  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res = sqw.queryT("select * from data where piefpaf = ?", {3});
  CHECK(res.size() == 1);
  CHECK(get<double>(res[0]["temperature"])==18.2);
  CHECK(get<int64_t>(res[0]["piefpaf"])==3);

  res = sqw.queryT("select sum(temperature) as s from data");
  CHECK(get<double>(res[0]["s"])==18.2);

  res = sqw.queryT("select user from data where piefpaf = ?", {42});
  CHECK(get<string>(res[0]["user"])=="ahu");

  res = sqw.queryT("select * from data order by piefpaf", {});
  CHECK(get<nullptr_t>(res[0]["temperature"])==nullptr);

  res = sqw.queryT("select piefpaf from data where piefpaf NOT NULL order by piefpaf");
  CHECK(res.size() == 4);
  CHECK(get<int64_t>(res[0]["piefpaf"])==2);
  CHECK(get<int64_t>(res[2]["piefpaf"])==7);

  res = sqw.queryT("select sum(poef) as s from metadata where poef > ?", {-2});
  CHECK(res.size() == 1);
  CHECK(get<int64_t>(res[0]["s"])==30);

  res = sqw.queryT("select sum(poef) as s from metadata where poef > ?", {-1});
  CHECK(res.size() == 1);
  CHECK(get<int64_t>(res[0]["s"])==31);

  unlink("testrunner-example.sqlite3");
}


TEST_CASE("test meta") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3", {{"piefpaf", "collate nocase"}});
    sqw.addValue({{"piefpaf", "Guus"}, {"poef", "guus"}});
  }

  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res = sqw.query("select count(1) as c from data where piefpaf = ?", {"GUUS"});
  CHECK(res.size() == 1);
  CHECK(res[0]["c"]=="1");

  res = sqw.query("select count(1) as c from data where poef = ?", {"GUUS"});
  CHECK(res.size() == 1);
  CHECK(res[0]["c"]=="0");
  
  unlink("testrunner-example.sqlite3");
}

TEST_CASE("test meta non-default table") {
  unlink("testrunner-example.sqlite3");
  {
    SQLiteWriter sqw("testrunner-example.sqlite3", {{"second", {{"piefpaf", "collate nocase"}}}});
    sqw.addValue({{"piefpaf", "Guus"}, {"poef", "guus"}}, "second");
    sqw.addValue({{"town", "Nootdorp"}, {"city", "Pijnacker-Nootdorp"}});
  }

  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res = sqw.query("select count(1) as c from second where piefpaf = ?", {"GUUS"});
  CHECK(res.size() == 1);
  CHECK(res[0]["c"]=="1");

  res = sqw.query("select count(1) as c from second where poef = ?", {"GUUS"});
  CHECK(res.size() == 1);
  CHECK(res[0]["c"]=="0");

  auto res2 = sqw.queryT("select count(1) as c from data where city = ?", {"pijnacker-nootdorp"});
  REQUIRE(res2.size() == 1);
  CHECK(get<int64_t>(res2[0]["c"]) == 0);

  res2 = sqw.queryT("select count(1) as c from data where city = ?", {"Pijnacker-Nootdorp"});
  REQUIRE(res2.size() == 1);
  CHECK(get<int64_t>(res2[0]["c"]) == 1);
  
  unlink("testrunner-example.sqlite3");
}

TEST_CASE("8 bit safe string") {
  unlink("testrunner-example.sqlite3");
  string bit8;
  for(int n=255; n; --n) {
    bit8.append(1, (char)n);
  }
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"binarystring", bit8}});
  }
  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res =sqw.queryT("select cast(binarystring as BLOB) as bstr from data");
  auto t = get<vector<uint8_t>>(res[0]["bstr"]);
  CHECK(string((char*)&t.at(0), t.size())==bit8);

  res =sqw.queryT("select binarystring as bstr from data");
  CHECK(get<string>(res.at(0).at("bstr")) == bit8);
  unlink("testrunner-example.sqlite3");
}

TEST_CASE("blob test") {
  unlink("testrunner-example.sqlite3");
  vector<uint8_t> bit8;
  for(int n=128; n; --n) {
    bit8.push_back(n);
  }
  for(int n=255; n > 128; --n) {
    bit8.push_back(n);
  }
  
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"binblob", bit8}});
  }
  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res =sqw.queryT("select binblob from data");
  CHECK(get<vector<uint8_t>>(res[0]["binblob"])==bit8);
  unlink("testrunner-example.sqlite3");
}

TEST_CASE("empty blob and string") {
  unlink("testrunner-example.sqlite3");
  vector<uint8_t> bit8;
  string leer;
  
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"vec", bit8}, {"str", leer}});
  }
  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res =sqw.queryT("select * from data");
  CHECK(get<string>(res[0]["str"])==leer);
  CHECK(get<vector<uint8_t>>(res[0]["vec"])==bit8);

  unlink("testrunner-example.sqlite3");
}

TEST_CASE("deal with constraint error") {
  unlink("testrunner-example.sqlite3");
  SQLiteWriter sqw("testrunner-example.sqlite3", {{"user", "UNIQUE"}});
  sqw.addValue({{"user", "ahu"}, {"test", 1}});
  
  REQUIRE_THROWS_AS(sqw.addValue({{"user", "ahu"}, {"test", 2}}), std::exception);
  // we previously had a bug where throwing such an exception would
  // leave a prepared statement in a bad state, which would lead
  // to subsequent bogus violations of constraints
  sqw.addValue({{"user", "ahu2"}, {"test", 1}});
  sqw.addValue({{"user", "ahu3"}, {"test", 1}});
  unlink("testrunner-example.sqlite3");
}


TEST_CASE("test foreign keys") {
  unlink("testrunner-example.sqlite3");
  SQLiteWriter sqw("testrunner-example.sqlite3",
                   {
                     {"posts", {{"id", "PRIMARY KEY NOT NULL"}} },
                     {"images", {{"postid", "NOT NULL REFERENCES posts(id) ON DELETE CASCADE"}}},
                   });;
  auto res = sqw.query("PRAGMA foreign_keys");
  CHECK(res.size()==1);
  CHECK(res[0]["foreign_keys"]=="1");
  
  

  sqw.addValue({{"id", "main"}, {"name", "main album"}}, "posts");
  sqw.addValue({{"id", "first"}, {"postid", "main"}}, "images");

  REQUIRE_THROWS_AS(sqw.addValue({{"id", "second"}}, "images"), std::exception);
  REQUIRE_THROWS_AS(sqw.addValue({{"id", "second"}, {"postid", "nosuchpost"}}, "images"), std::exception);

  sqw.query("delete from posts where id=?", {"main"});
  auto res2 = sqw.queryT("select count(1) as c from images");
  REQUIRE(res2.size() == 1);
  CHECK(get<int64_t>(res2[0]["c"])==0);
  
}

TEST_CASE("insert or replace") {
  unlink("insertorreplace.sqlite3");
  SQLiteWriter sqw("insertorreplace.sqlite3", {
      {
	"data", {{"id", "PRIMARY KEY"}}}
    });
  sqw.addValue({{"id", 1}, {"user", "ahu"}});
  REQUIRE_THROWS_AS(  sqw.addValue({{"id", 1}, {"user", "jhu"}}), std::exception);
  auto res = sqw.queryT("select user from data where id=?", {1});
  CHECK(res.size() == 1);
  
  sqw.addOrReplaceValue({{"id", 1}, {"user", "harry"}});

  res = sqw.queryT("select user from data where id=?", {1});
  REQUIRE(res.size() == 1);
  CHECK(get<string>(res[0]["user"]) == "harry");

  REQUIRE_THROWS_AS(  sqw.addValue({{"id", 1}, {"user", "jhu"}}), std::exception);
  unlink("insertorreplace.sqlite3");
}

TEST_CASE("readonly test") {
  string fname="readonly-test.sqlite3";
  unlink(fname.c_str());

  {
    SQLiteWriter sqw(fname);
    sqw.addValue({{"some", "stuff"}, {"val", 1234.0}});
  }

  SQLiteWriter ro(fname, SQLWFlag::ReadOnly);
  auto res = ro.queryT("select * from data where some='stuff'");
  REQUIRE(res.size() == 1);

  REQUIRE_THROWS_AS(ro.addValue({{"some", "more"}}), std::exception);
  res = ro.queryT("select count(1) c from data");
  REQUIRE(res.size() == 1);
  CHECK(get<int64_t>(res[0]["c"]) == 1);
  unlink(fname.c_str());
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
