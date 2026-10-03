#pragma once
#include"mysql/mysql.h"
#include<string>
#include<iostream>
#include<memory>
#include<vector>
#include<utility>
#include"Timestamp.h"
#include"Log.h"
#include"nlohmann/json.hpp"
using json = nlohmann::json;
class MySql{
public:
MySql();
~MySql();
bool insert(std::string word);
json find(std::string word);
bool update(std::string word);
void settime(){time.setnow();}
long long gettime(){return time.alive();}
MYSQL* outinit(){return mysql;}
std::string escape(const std::string& input);
bool isAlive();
bool reconnection();
std::string quote(const std::string& v) { return "'" + escape(v) + "'"; }

private:
MySql(const MySql&) = delete;
MySql& operator=(const MySql&) = delete;
MYSQL* mysql;
Timestamp time;
json find_json(const std::shared_ptr<MYSQL_RES>& res)
{
    json arr = json::array();
    if (!res) return arr;                                

    MYSQL_FIELD*   fields = mysql_fetch_fields(res.get());
    const unsigned n      = mysql_num_fields(res.get()); 
    std::vector<char> kind(n, 'S'); 
    for (unsigned i = 0; i < n; ++i) {
        switch (fields[i].type) { 
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_YEAR:          kind[i] = 'I'; break;
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_DECIMAL:                                
        case MYSQL_TYPE_NEWDECIMAL:    kind[i] = 'R'; break;
        default:                       kind[i] = 'S'; break;      
        }
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res.get()))) {            
        unsigned long* lens = mysql_fetch_lengths(res.get());
        json obj = json::object();
        for (unsigned i = 0; i < n; ++i) {
            const std::string key = fields[i].name;

            if (!row[i]) { obj[key] = nullptr; continue; }

            const std::string v(row[i], lens[i]);

            if      (kind[i] == 'I') obj[key] = std::atoll(v.c_str());
            else if (kind[i] == 'R') obj[key] = std::atof(v.c_str());
            else                     obj[key] = v;
        }
        arr.push_back(std::move(obj));
    }
    return arr;
}
};


class SqlValue {
public:
    SqlValue(const std::string& s) : kind_(KStr),  s_(s) {}
    SqlValue(const char* s)        : kind_(KStr),  s_(s) {}
    SqlValue(int n)                : kind_(KInt),  n_(n) {}
    SqlValue(unsigned n)           : kind_(KInt),  n_(n) {}
    SqlValue(long long n)          : kind_(KInt),  n_(n) {}
    SqlValue(size_t n)             : kind_(KInt),  n_((long long)n) {}
    SqlValue(double v)             : kind_(KReal), d_(v) {}

    static SqlValue Null() { SqlValue v; return v; }

    std::string Literal(MySql& db) const {
        switch (kind_) {
        case KStr:  return db.quote(s_);              // 'xxx'
        case KInt:  return std::to_string(n_);        // 123
        case KReal: return std::to_string(d_);        // 1.500000
        case KNull: return "NULL";
        }
        return "NULL";
    }
private:
    enum Kind { KNull, KStr, KInt, KReal };
    SqlValue() : kind_(KNull) {}
    Kind        kind_;
    std::string s_;
    long long   n_ = 0;
    double      d_ = 0.0;
};

using SqlCols = std::vector<std::pair<std::string, SqlValue>>;

struct SqlCond {
    std::string col, op;
    SqlValue    val;

    SqlCond(std::string c, SqlValue v)                       // 默认 =
        : col(std::move(c)), op("="), val(std::move(v)) {}
    SqlCond(std::string c, std::string o, SqlValue v)        // 指定运算符
        : col(std::move(c)), op(std::move(o)), val(std::move(v)) {}
};
using SqlConds = std::vector<SqlCond>;

inline std::string SqlId(const std::string& n) { return "`" + n + "`"; }

inline std::string SqlWhere(MySql& db, const SqlConds& w) {
    if (w.empty()) return "";
    std::string s = " WHERE ";
    for (size_t i = 0; i < w.size(); ++i) {
        if (i) s += " AND ";
        s += SqlId(w[i].col) + " " + w[i].op + " " + w[i].val.Literal(db);
    }
    return s;
}

inline std::string SqlSet(MySql& db, const SqlCols& c) {
    std::string s;
    for (size_t i = 0; i < c.size(); ++i) {
        if (i) s += ", ";
        s += SqlId(c[i].first) + " = " + c[i].second.Literal(db);
    }
    return s;
}

inline std::string BuildInsert(MySql& db, const std::string& table, const SqlCols& cols) {
    if (cols.empty()) return "";
    std::string names, values;
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) { names += ", "; values += ", "; }
        names  += SqlId(cols[i].first);
        values += cols[i].second.Literal(db);
    }
    return "INSERT INTO " + SqlId(table) + " (" + names + ") VALUES (" + values + ")";
}

inline std::string BuildSelect(MySql& db, const std::string& table,
                               const std::vector<std::string>& cols = {},
                               const SqlConds& where      = {},
                               const std::string& orderBy = {},
                               const std::string& tail    = {})
{
    std::string colList = "*";
    if (!cols.empty()) {
        colList.clear();
        for (size_t i = 0; i < cols.size(); ++i) {
            if (i) colList += ", ";
            colList += SqlId(cols[i]);
        }
    }
    std::string s = "SELECT " + colList + " FROM " + SqlId(table) + SqlWhere(db, where);
    if (!orderBy.empty()) s += " ORDER BY " + orderBy;
    if (!tail.empty())    s += " " + tail;
    return s;
}

// ── UPDATE ────────────────────────────────────────────────────
inline std::string BuildUpdate(MySql& db, const std::string& table,
                               const SqlCols& setCols, const SqlConds& where,
                               bool allowEmptyWhere = false)
{
    if (setCols.empty()) return "";
    if (where.empty() && !allowEmptyWhere) {
        LOG_ERROR("BuildUpdate: 拒绝生成没有 WHERE 的 UPDATE（会改整张表）");
        return "";
    }
    return "UPDATE " + SqlId(table) + " SET " + SqlSet(db, setCols) + SqlWhere(db, where);
}

// ── DELETE ────────────────────────────────────────────────────
inline std::string BuildDelete(MySql& db, const std::string& table,
                               const SqlConds& where, bool allowEmptyWhere = false)
{
    if (where.empty() && !allowEmptyWhere) {
        LOG_ERROR("BuildDelete: 拒绝生成没有 WHERE 的 DELETE（会清空整张表）");
        return "";
    }
    return "DELETE FROM " + SqlId(table) + SqlWhere(db, where);
}

