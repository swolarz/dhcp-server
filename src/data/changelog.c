#include "changelog.h"
#include "data/db.h"
#include "sqldiff.h"
#include "context.h"
#include "utils/log/log.h"


static struct logger* loggr() {
	return context_get_logger();
}


int sqldiff_exists(sqlite3* db, sql_diff* diff) {
	const char* select_diff_sql = 
		"select 1 from sqlchangelog "
			"where id = ? ";

	sqlite3_stmt* query;
	int err;

	err = prepare_sql_stmt(db, select_diff_sql, &query);
	if (err) return -1;

	err = sqlite3_bind_int(query, 1, diff->id);
	if (err != SQLITE_OK) {
		log_error(loggr(), "SQLDIFF", "Failed to bind sql diff id (%d) to query", diff->id);
		sqlite3_finalize(query);

		return -1;
	}

	log_debug(loggr(), "SQLDIFF", "Checking log for sqldiff (id = %d): %s", diff->id, diff->sql);

	err = sqlite3_step(query);
	int res = -1;

	if (err == SQLITE_DONE)
		res = 0;
	else if (err == SQLITE_ROW)
		res =  1;
	else {
		res = -1;
		log_error(loggr(), "SQLDIFF", "Query execution failed: %s", sqlite3_errmsg(db));
	}
	
	sqlite3_finalize(query);

	return res;
}

int sqldiff_apply(sqlite3* db, sql_diff* diff) {
	int err = exec_immediate_sql_stmt(db, diff->sql);
	if (err) return -1;

	const char* update_sqllog_sql =
		"insert into sqlchangelog (id, sql) "
			"values (?, ?) ";

	sqlite3_stmt* stmt;

	err = prepare_sql_stmt(db, update_sqllog_sql, &stmt);
	if (err) return -1;

	err =  sqlite3_bind_int(stmt, 1, diff->id);

	if (err == SQLITE_OK)
		err = sqlite3_bind_text(stmt, 2, diff->sql, -1, NULL);

	if (err != SQLITE_OK) {
		log_error(loggr(), "SQLDIFF", "Failed to bind sqldiff insert parameters: %s", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		
		return -1;
	}

	err = exec_sql_stmt(db, stmt);
	if (err) return -1;

	return 0;
}

int ensure_changelog_table(sqlite3* db) {
	const char* create_sql =
			"create table if not exists sqlchangelog ( "
				"id integer primary key, "
				"sql text not null "
			")";

	return exec_immediate_sql_stmt(db, create_sql);
}

int migrate_sql_changelog(sqlite3* db) {
	int err = 0;

	log_info(loggr(), "SQLDIFF", "Migrating sql changelog...");
	
	err = ensure_changelog_table(db);
	if (err) return -1;

	int changelog_size = sizeof(sqldiff_changelog) / sizeof(sql_diff);
	int i;
	
	for (i = 0; i < changelog_size; ++i) {
		sql_diff* diff = &sqldiff_changelog[i];

		err = sqldiff_exists(db, diff);
		if (err < 0) return -1;

		if (err == 0) {
			err = sqldiff_apply(db, diff);
			if (err) return -1;
		}
	}
	
	return 0;
}


