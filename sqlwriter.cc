#include "sqlwriter.hh"
#include <algorithm>
#include <unistd.h>
#include "sqlite3.h"
using namespace std;

MiniSQLite::MiniSQLite(std::string_view fname)
{
  if ( sqlite3_open(&fname[0], &d_sqlite)!=SQLITE_OK ) {
    throw runtime_error("Unable to open "+(string)fname+" for sqlite");
  }
  exec("PRAGMA journal_mode='wal'");
  d_notable=getSchema().empty();
}



//! Get field names and types from a table
vector<pair<string,string> > MiniSQLite::getSchema()
{
  vector<pair<string,string>> ret;
  
  auto rows = exec("SELECT cid,name,type FROM pragma_table_xinfo('data')");


  for(const auto& r : rows) {
    ret.push_back({r[1], r[2]});
  }
  sort(ret.begin(), ret.end(), [](const auto& a, const auto& b) {
    return a.first < b.first;
  });
   
  return ret;
}

int MiniSQLite::helperFunc(void* ptr, int cols, char** colvals, char** colnames)
{
  vector<string> row;
  row.reserve(cols);
  for(int n=0; n < cols ; ++n)
    row.push_back(colvals[n]);
  ((MiniSQLite*)ptr)->d_rows.push_back(row);
  return 0;
}

vector<vector<string>> MiniSQLite::exec(std::string_view str)
{
  char *errmsg;
  std::string errstr;
  //  int (*callback)(void*,int,char**,char**)
  d_rows.clear();
  int rc = sqlite3_exec(d_sqlite, &str[0], helperFunc, this, &errmsg);
  if (rc != SQLITE_OK) {
    errstr = errmsg;
    sqlite3_free(errmsg);
    throw std::runtime_error("Error executing sqlite3 query '"+(string)str+"': "+errstr);
  }
  return d_rows; 
}

void MiniSQLite::bindPrep(int idx, bool value) {   sqlite3_bind_int(d_stmt, idx, value ? 1 : 0);   }
void MiniSQLite::bindPrep(int idx, int value) {   sqlite3_bind_int(d_stmt, idx, value);   }
void MiniSQLite::bindPrep(int idx, uint32_t value) {   sqlite3_bind_int64(d_stmt, idx, value);   }
void MiniSQLite::bindPrep(int idx, long value) {   sqlite3_bind_int64(d_stmt, idx, value);   }
void MiniSQLite::bindPrep(int idx, unsigned long value) {   sqlite3_bind_int64(d_stmt, idx, value);   }
void MiniSQLite::bindPrep(int idx, long long value) {   sqlite3_bind_int64(d_stmt, idx, value);   }
void MiniSQLite::bindPrep(int idx, unsigned long long value) {   sqlite3_bind_int64(d_stmt, idx, value);   }
void MiniSQLite::bindPrep(int idx, double value) {   sqlite3_bind_double(d_stmt, idx, value);   }
void MiniSQLite::bindPrep(int idx, const std::string& value) {   sqlite3_bind_text(d_stmt, idx, value.c_str(), value.size(), SQLITE_TRANSIENT);   }


void MiniSQLite::prepare(string_view str)
{
  if(d_stmt) {
    sqlite3_finalize(d_stmt);
    d_stmt = 0;
  }
  const char* pTail;
  
  if (sqlite3_prepare_v2(d_sqlite, &str[0], -1, &d_stmt, &pTail ) != SQLITE_OK) {
    throw runtime_error("Unable to prepare query "+(string)str);
  }
}

void MiniSQLite::execPrep()
{
  int rc;
  for(;;) {
    rc = sqlite3_step(d_stmt); // XXX this needs to be an error checking loop
    if(rc == SQLITE_DONE)
      break;
    else
      throw runtime_error("Sqlite error: "+std::to_string(rc));
  }
  rc= sqlite3_reset(d_stmt);
  if(rc != SQLITE_OK)
    throw runtime_error("Sqlite error: "+std::to_string(rc));
  sqlite3_clear_bindings(d_stmt);
}

void MiniSQLite::begin()
{
  d_intransaction=true;
  exec("begin");
}
void MiniSQLite::commit()
{
  d_intransaction=false;
  exec("commit");
}

void MiniSQLite::cycle()
{
  exec("commit;begin");
}

//! Add a column to a atable with a certain type
void MiniSQLite::addColumn(string_view name, string_view type)
{
  // SECURITY PROBLEM - somehow we can't do prepared statements here
  
  if(d_notable) {
#if SQLITE_VERSION_NUMBER >= 3037001
    exec("create table if not exists data ( '"+(string)name+"' "+(string)type+" ) STRICT");
#else
    exec("create table if not exists data ( '"+(string)name+"' "+(string)type+" )");
#endif
    d_notable=false;
  } else {
    exec("ALTER table data add column \""+string(name)+ "\" "+string(type));
  }
}



void SQLiteWriter::commitThread()
{
  int n=0;
  while(!d_pleasequit) {
    usleep(50000);
    if(!(n%20)) {
      std::lock_guard<std::mutex> lock(d_mutex);
      d_db.cycle();
    }
    n++;
  }
  //  cerr<<"Thread exiting"<<endl;
}

bool SQLiteWriter::haveColumn(std::string_view name)
{
  // this could be more efficient somehow
  pair<string, string> cmp{name, std::string()};
  return binary_search(d_columns.begin(), d_columns.end(), cmp,
                       [](const auto& a, const auto& b)
                       {
                         return a.first < b.first;
                       });

}


void SQLiteWriter::addValue(const initializer_list<std::pair<const char*, var_t>>& values)
{
  addValueGeneric(values);
}

void SQLiteWriter::addValue(const std::vector<std::pair<const char*, var_t>>& values)
{
  addValueGeneric(values);
}


template<typename T>
void SQLiteWriter::addValueGeneric(const T& values)
{
  std::lock_guard<std::mutex> lock(d_mutex);
  if(!d_db.isPrepared() || !equal(values.begin(), values.end(),
                            d_lastsig.cbegin(), d_lastsig.cend(),
                            [](const auto& a, const auto& b)
  {
    return a.first == b;
  })) {
    //    cout<<"Starting a new prepared statement"<<endl;
    string q = "insert into data (";
    string qmarks;
    bool first=true;
    for(const auto& p : values) {
      if(!haveColumn(p.first)) {
        if(std::get_if<double>(&p.second))
          d_db.addColumn(p.first, "REAL");
        else if(std::get_if<string>(&p.second))
          d_db.addColumn(p.first, "TEXT");
        else 
          d_db.addColumn(p.first, "INT");
      }
      if(!first) {
        q+=", ";
        qmarks += ", ";
      }
      first=false;
      q+="'"+string(p.first)+"'";
      qmarks +="?";
    }
    q+= ") values ("+qmarks+")";

    d_db.prepare(q);
    
    d_lastsig.clear();
    for(const auto& p : values)
      d_lastsig.push_back(p.first);
  }
  else
    ;//cout<<"We can reuse old statement"<<endl;
  
  int n = 1;
  for(const auto& p : values) {
    std::visit([this, &n](auto&& arg) {
      d_db.bindPrep(n, arg);
    }, p.second);
    n++;
  }
  d_db.execPrep();
}

MiniSQLite::~MiniSQLite()
{
  // needs to close down d_sqlite3
  if(d_intransaction)
    commit();

  if(d_stmt) // this could generate an error, but there is nothing we can do with it
    sqlite3_finalize(d_stmt);
  
  sqlite3_close(d_sqlite); // same
}
