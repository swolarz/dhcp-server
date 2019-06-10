#ifndef __H_DHCP_STORE_DB
#define __H_DHCP_STORE_DB

#include "sqlite3.h"


sqlite3* init_sqlite_connection();
void close_sqlite_connection(sqlite3* db);

int prepare_sql_stmt(sqlite3* db, const char* sql, sqlite3_stmt** stmt);
int exec_sql_stmt(sqlite3* db, sqlite3_stmt* stmt);
int exec_immediate_sql_stmt(sqlite3* db, const char* sql);

int begin_sql_transaction(sqlite3* db);
int commit_sql_transaction(sqlite3* db);
int rollback_sql_transaction(sqlite3* db);


#endif // __H_DHCP_STORE_DB
