#ifndef __H_DATA_CHANGELOG
#define __H_DATA_CHANGELOG

#include <sqlite3.h>


typedef struct {
	int id;
	const char* sql;

} sql_diff;


int migrate_sql_changelog(sqlite3* db);


#endif // __H_DATA_CHANGELOG
