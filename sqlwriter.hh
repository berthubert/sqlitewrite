#pragma once
#include <string>
#include <vector>
#include <variant>
#include <mutex>
#include <thread>
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

class SQLiteWriter
{
private:
  typedef std::variant<double, int32_t, uint32_t, int64_t, std::string> var_t;
public:
  explicit SQLiteWriter(std::string_view fname) : d_db(fname, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE)
  {
    d_columns = getSchema("data");
    //    for(const auto& c : d_columns)
    //      cout <<c.first<<"\t"<<c.second<<endl;

    d_db.exec("PRAGMA journal_mode='wal'");
    begin(); // open the transaction
    d_thread = std::thread(&SQLiteWriter::commitThread, this);
  }
  void addValue(const std::initializer_list<std::pair<const char*, var_t>>& values);
  ~SQLiteWriter()
  {
    d_pleasequit=true;
    d_thread.join();
    if(d_transaction) {
      //      cerr<<"Had open transaction"<<endl;
      commit();
    }
    if(d_statement)
      d_statement.reset();
  }
private:
  void begin()
  {
    d_transaction = std::make_unique<SQLite::Transaction>(d_db);
  }
  void commit()
  {
    if(d_transaction) {
      d_transaction->commit();
      d_transaction.reset();
    }
  }
  void cycle()
  {
    commit();
    begin();
  }

  void commitThread();
  std::vector<std::pair<std::string,std::string> > getSchema(std::string_view table);
  bool d_pleasequit{false};
  std::thread d_thread;
  std::mutex d_mutex;  
  SQLite::Database d_db;
  std::unique_ptr<SQLite::Statement> d_statement;
  std::vector<std::pair<std::string, std::string>> d_columns;
  bool haveColumn(std::string_view name);
  void addColumn(std::string_view name, std::string_view type);
  std::unique_ptr<SQLite::Transaction> d_transaction;
  std::vector<std::string> d_lastsig;
};
