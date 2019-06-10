#ifndef __H_DATA_SQLDIFF
#define __H_DATA_SQLDIFF


sql_diff sqldiff_changelog[] = {
	{
		0,
		"create table test ("
			"id integer primary key"
		");"
	},
	{
		1,
		"insert into test (id) values (1);"
	}
};


#endif //  __H_DATA_SQLDIFF
