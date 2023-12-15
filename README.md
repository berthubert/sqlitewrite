# sqlitewriter
csv-like storage to sqlite and PostgreSQL. Easy to use, type safe:

```C++
SQLiteWriter sqw("example.sqlite3");

for(int n = 0 ; n < 1000000; ++n) {
  sqw.addValue({{"pief", n}, {"poef", 1.1*n}, {"paf", "bert"}});
}

sqw.addValue({{"timestamp", 1234567890}});
```

This will create `example.sqlite3` if needed. Also, if needed it will create
a table called `data`. The sqlite3 file is set to WAL mode, the table is set
to STRICT mode:

```
sqlite> PRAGMA journal_mode;
wal

sqlite> .schema
CREATE TABLE data ( pief INT , "poef" REAL, "paf" TEXT, "timestamp" INT) STRICT;
```

The `addValue()` method can be used to pass an arbitrary number of fields.
Columns will automatically be generated in the table on their first use. 

Note that strings are stored as TEXT which means they benefit from utf-8 processing. If you don't want that, store `vector<uint8_t>`, which will end up as a blob.

You can also add data to other tables than `data` like this:

```C++
// put data in table logins
sqw.addValue({{"timestamp", 1234567890}}, "logins");
```

SQLiteWriter batches all writes in transactions. The transaction is cycled
every second. Because WAL mode is used, readers can be active without
interrupting writes to the file.

In case you are trying to read from the file while sqlwriter is trying to
write to it, a busy handler is implemented. This will not block your main
process, since writes are performed from a writing thread.

Prepared statements are automatically created and reused as long as the same
fields are passed to `addValue()`. If different fields are passed, a new
prepared statement is made.

The code above does around 550k inserts/second. This could likely be
improved further, but it likely suffices for most usecases.

# Compiling
Make sure you have a C++ compiler installed, plus CMake and of course the SQLite3 development files. In addition, the tests also exercise the optional JSON helpers. These are part of nlohmann-json3-dev or find the single include file needed [here](https://github.com/nlohmann/json/releases).

```
git clone https://github.com/berthubert/sqlitewrite.git 
cd sqlitewrite
cmake .
make
./testrunner
```

# Using in your project
Simply drop `sqlitewriter.cc` and `sqlitewriter.hh` in your project, and
link in libsqlite3-dev (or if needed, the amalgamated sqlite3 source). If you want to benefit from the JSON helpers below, also compile in `jsonhelper.cc`, and include `jsonhelper.hh`.

# PostgreSQL version
Slightly less tested, but you can get the same API if you add psqlwriter.{cc,hh} and minipsql.{cc,hh} to your project. The class is called `PSQLWriter`, and instead of a filename you pass a connection string.

Despite valiant efforts, the PostgreSQL version is 5 times slower than the SQLite version (at least). However, if you want your logged data to be available remotely, this is still a very good option.

The reason for the relative slowness is mostly down to the interprocess/network latency. The code attempts to batch a lot and use efficient APIs, but simply serializing all the data eats up a lot of CPU.

# The (sqlite) writer can also read
Although it is a bit of anomaly, you can also use sqlitewriter to perform queries:

```
SQLiteWriter sqw("example.sqlite3");
auto res = sqw.query("select * from data where pief = ?", {123});
cout << res.at(0)["pief"] << endl; // should print 123
```

This returns a vector of rows, each represented by an `unordered_map<string,string>`. Alternatively you can use `queryT` which returns `std::variant<int64_t, double, string, nullptr_t, vector<uint8_t>>` values, which means you can benefit from the typesafety. Note that all integer numbers end  up as signed int64_t in this variant union.

Also note that these functions could very well not be coherent with `addValue()` because of buffering. This is by design.  The `query` function is very useful for getting the value of counters for example, so you can use these for subsequent logging. But don't count on a value you just wrote with `addValue()` to appear in an a `query()` immediately. 

To be on the safe side, don't interleave calls to `query()` with calls to `addValue()`.

## JSON helper
It is often convenient to turn your query results into JSON. The helpers `packResultJson` and `packResultJsonStr` in `jsonhelper.cc` benefit from the typesafety provided by `queryT` to create JSON that knows that 1.0 is not "1.0":

```C++
  {
    SQLiteWriter sqw("testrunner-example.sqlite3");
    sqw.addValue({{"pief",1}});
    sqw.addValue({{"paf",12.0}});
    sqw.addValue({{"user", "ahu"}, {"paf", 14}});
    sqw.addValue({{"user", "jhu"}, {"paf", 14.23}, {"pief", 99}});
  }
  // best way to guarantee that you can query the data you inserted 
  // is by closing the connection
  SQLiteWriter sqw("testrunner-example.sqlite3");
  auto res = sqw.queryT("select * from data");

  cout << packResultsJsonString(res) << endl;
```

Slightly reformatted, this prints:
```
[
 {"pief":1},
 {"paf":12.0},
 {"paf":14.0, "user":"ahu"},
 {"paf":14.23, "pief":99, "user":"jhu"}
]
```

# Advanced features
## Per-field metadata
SQLiteWriter creates populates tables for you automatically, setting the correct type per field. But sometimes you want to add some additional instructions. For example, you might want to declare that a field is case-insensitive. To do so, you can add metadata per field, like this:

```C++
SQLiteWriter sqw("example.sqlite3", {{"name", "collate nocase"}});
```

This will lead SQLiteWriter to add 'collate nocase' when the 'name' field is created for the main 'data' table. To specify for specific tables, use this syntax:

```C++
SQLiteWriter sqw("example.sqlite.sqlite3", {{"second", 
    {{"user", "UNIQUE"}}
    }});
```

Note that this is also the only time when this metadata is used. If you have a pre-existing database that already has a 'name' field, it will not be changed to 'collate nocase'.


# Status
I use it as infrastructure in some of my projects. Seems to work well. 
