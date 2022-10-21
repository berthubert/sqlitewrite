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

# Status
Proof of concept.
