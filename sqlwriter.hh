#pragma once
#include <string>
#include <vector>
#include <variant>
#include <mutex>
#include <thread>
#include <iostream>

struct sqlite3;
struct sqlite3_stmt;

class MiniSQLite
{
public:
  MiniSQLite(std::string_view fname);
  ~MiniSQLite();
  std::vector<std::pair<std::string, std::string>> getSchema();
  void addColumn(std::string_view name, std::string_view type);
  std::vector<std::vector<std::string>> exec(std::string_view query);
  void prepare(std::string_view str);
  void bindPrep(int idx, bool value);
  void bindPrep(int idx, int value);
  void bindPrep(int idx, uint32_t value);
  void bindPrep(int idx, long value);
  void bindPrep(int idx, unsigned long value);
  void bindPrep(int idx, long long value); 
  void bindPrep(int idx, unsigned long long value);
  void bindPrep(int idx, double value);
  void bindPrep(int idx, const std::string& value);
  void execPrep(); 
  void begin();
  void commit();
  void cycle();
  bool isPrepared() const
  {
    return d_stmt != nullptr;
  }
private:
  sqlite3* d_sqlite;
  sqlite3_stmt* d_stmt{nullptr};
  
  std::vector<std::vector<std::string>> d_rows; // for exec()
  static int helperFunc(void* ptr, int cols, char** colvals, char** colnames);
  bool d_notable;
  bool d_intransaction{false};
};

class SQLiteWriter
{

public:
  explicit SQLiteWriter(std::string_view fname) : d_db(fname)
  {
    d_columns = d_db.getSchema();
    //    for(const auto& c : d_columns)
    //      cout <<c.first<<"\t"<<c.second<<endl;

    d_db.exec("PRAGMA journal_mode='wal'");
    d_db.begin(); // open the transaction
    d_thread = std::thread(&SQLiteWriter::commitThread, this);
  }
  typedef std::variant<double, int32_t, uint32_t, int64_t, std::string> var_t;
  void addValue(const std::initializer_list<std::pair<const char*, var_t>>& values);
  void addValue(const std::vector<std::pair<const char*, var_t>>& values);
  template<typename T>
  void addValueGeneric(const T& values);
  ~SQLiteWriter()
  {
    //    std::cerr<<"Destructor called"<<std::endl;
    d_pleasequit=true;
    d_thread.join();
  }

private:
  void commitThread();
  bool d_pleasequit{false};
  std::thread d_thread;
  std::mutex d_mutex;  
  MiniSQLite d_db;
  std::vector<std::pair<std::string, std::string>> d_columns;
  std::vector<std::string> d_lastsig;
  bool haveColumn(std::string_view name);

};
