#ifndef __H_DHCP_STORE
#define __H_DHCP_STORE

struct db_connection;

struct db_connection* init_db_connection();
void cleanup_db_connection(struct db_connection* db_conn);

#endif // __H_DHCP_STORE
