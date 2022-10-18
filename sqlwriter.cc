#include "sqlwriter.hh"
#include <algorithm>
#include <unistd.h>
using namespace std;

//! Get field names and types from a table
vector<pair<string,string> > SQLiteWriter::getSchema(string_view table)
{
  SQLite::Statement   query(d_db, "SELECT cid,name,type FROM pragma_table_xinfo(?)");
  
  query.bind(1, (string)table);

  vector<pair<string,string>> ret;
  while (query.executeStep()) {
    ret.push_back({query.getColumn(1).getText(), query.getColumn(2).getText()});
  }
  sort(ret.begin(), ret.end(), [](const auto& a, const auto& b) {
    return a.first < b.first;
  });
   
  return ret;
}


void SQLiteWriter::commitThread()
{
  int n=0;
  while(!d_pleasequit) {
    usleep(50000);
    if(!(n%20)) {
      std::lock_guard<std::mutex> lock(d_mutex);
      cycle();
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

//! Add a column to a atable with a certain type
void SQLiteWriter::addColumn(string_view name, string_view type)
{
  // SECURITY PROBLEM - somehow we can't do prepared statements here
  
  if(d_columns.empty()) {
    //    cout<<"Creating table with "<<name<<" as initial column"<<endl;
    SQLite::Statement c(d_db, "create table if not exists data ( "+(string)name+" "+(string)type+" ) STRICT");
    c.exec();
  } else {
    //    cout<<"Adding "<<name<<" as column with type "<<type<<endl;
    SQLite::Statement query(d_db, "ALTER table data add column \""+string(name)+ "\" "+string(type));
    query.exec();
  }
  d_columns = getSchema("data");
      
}


void SQLiteWriter::addValue(const initializer_list<std::pair<const char*, var_t>>& values)
{
  std::lock_guard<std::mutex> lock(d_mutex);
  if(!d_statement || !equal(values.begin(), values.end(),
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
          addColumn(p.first, "REAL");
        else if(std::get_if<string>(&p.second))
          addColumn(p.first, "TEXT");
        else 
          addColumn(p.first, "INT");
      }
      if(!first) {
        q+=", ";
        qmarks += ", ";
      }
      first=false;
      q+=p.first;
      qmarks +="?";
    }
    q+= ") values ("+qmarks+")";

    d_statement = make_unique<SQLite::Statement>(d_db, q);
    d_lastsig.clear();
    for(const auto& p : values)
      d_lastsig.push_back(p.first);
  }
  else
    ;//cout<<"We can reuse old statement"<<endl;
  
  int n = 1;
  for(const auto& p : values) {
    std::visit([this, &n](auto&& arg) {
      d_statement->bind(n, arg);
    }, p.second);
    n++;
  }
  d_statement->exec();
  d_statement->reset();
}

