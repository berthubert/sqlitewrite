# sqlitewrite
csv-like storage to sqlite. Easy to use, type safe:

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
```
git clone https://github.com/berthubert/sqlitewrite.git 
cd sqlitewrite
cmake .
make
```

# Using in your project
Simply drop `sqlitewriter.cc` and `sqlitewriter.hh` in your project, and
link in libsqlite3-dev.

# PostgreSQL version
Slightly less tested, but you can get the same API if you add psqlwriter.{cc,hh} and minipsql.{cc,hh} to your project.

Despite vailiant efforts, the PostgreSQL version is 5 times slower than the SQLite version (at least). However, if you want your logged data to be available remotely, this is still a very good option.

# The (sqlite) writer can also read
Although it is a bit of anomaly, you can also use sqlitewriter to perform queries:

```
SQLiteWriter sqw("example.sqlite3");
auto res = sqw.query("select * from data where pief = ?", {123});
cout << res.at(0)["pief"] << endl; // should print 123
```

This returns a vector of rows, each represented by an `unordered_map<string,string>`. 

Note that this function could very well not be coherent with `addValue()` because of buffering. This is by design.  The `query` function is very useful for getting the value of counters for example, so you can use these for subsequent logging. But don't count on a value you ust wrote with `addValue()` to appear in an a `query()` immediately. 

To be on the safe side, don't interleave calls to `query()` with calls to `addValue()`.
# Status
I use it as infrastructure in some of my projects. Seems to work well. 
