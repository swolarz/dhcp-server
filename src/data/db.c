#include "db.h"
#include "context.h"
#include "changelog.h"
#include "utils/log/log.h"
#include "utils/user.h"

#include <string.h>
#include <sqlite3.h>


static struct logger* loggr() {
	return context_get_logger();
}


const char* database_path() {
	static char path_buff[256];
	snprintf(path_buff, 255, "%s/dhcpv2.db", get_user_home());

	return path_buff;
}

int exec_immediate_sql_stmt(sqlite3* db, const char* sql) {
	log_debug(loggr(), "DB", "Executing SQL statement: %s", sql);

	char* errmsg = NULL;
	int err = sqlite3_exec(db, sql, 0, 0, &errmsg);

	if (err != SQLITE_OK) {
		log_error(loggr(), "DB", "Statement execution failed: %s", errmsg);
		sqlite3_free(errmsg);

		return -1;
	}

	return 0;
}

int prepare_sql_stmt(sqlite3* db, const char* sql, sqlite3_stmt** stmt) {
	int err = sqlite3_prepare_v2(db, sql, strlen(sql) + 1, stmt, NULL);

	if (err != SQLITE_OK) {
		log_error(loggr(), "DB", "Failed to prepare SQL statement (%s): %s", sql, sqlite3_errmsg(db));
		return -1;
	}

	return 0;
}

int exec_sql_stmt(sqlite3* db, sqlite3_stmt* stmt) {
	log_debug(loggr(), "DB", "Executing SQL statement: %s", sqlite3_sql(stmt));

	int err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_ROW && err != SQLITE_DONE) {
		log_error(loggr(), "DB", "SQL statement execution failed: %s", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);

		return -1;
	}

	return 0;
}

int begin_sql_transaction(sqlite3* db) {
	return exec_immediate_sql_stmt(db, "begin");
}

int commit_sql_transaction(sqlite3* db) {
	return exec_immediate_sql_stmt(db, "commit");
}

int rollback_sql_transaction(sqlite3* db) {
	return exec_immediate_sql_stmt(db, "rollback");
}

sqlite3* init_sqlite_connection() {
	sqlite3* db;
	int err;

	log_info(loggr(), "DB", "Connnecting to database at: %s", database_path());

	err = sqlite3_open(database_path(), &db);
	if (err != SQLITE_OK) {
		log_error(loggr(), "DB", "Failed to open database: %s", sqlite3_errmsg(db));
		sqlite3_close(db);

		return NULL;
	}

	err = migrate_sql_changelog(db);
	if (err != 0) {
		log_error(loggr(), "DB", "Failed to migrate database");
		sqlite3_close(db);

		return NULL;
	}

	return db;
}

void close_sqlite_connection(sqlite3* db) {
	log_info(loggr(), "DB", "Closing database connection...");
	sqlite3_close(db);
}

