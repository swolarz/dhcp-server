#include "store.h"
#include "common.h"
#include "context.h"
#include "data/db.h"
#include "utils/net.h"
#include "utils/log/log.h"

#include <stdlib.h>
#include <sqlite3.h>


static struct logger* loggr() {
	return context_get_logger();
}


typedef struct db_connection {
	sqlite3* db;

} db_connection;


db_connection* init_db_connection() {
	log_info(loggr(), "STORE", "Initializing database connection...");

	sqlite3* db = init_sqlite_connection();
	
	if (db == NULL) {
		log_error(loggr(), "STORE", "Database was not initialized succesfully");
		return NULL;
	}

	int err = migrate_database(db);
	if (err) {
		log_error(loggr(), "STORE", "Database was not migrated succesfully");
		close_sqlite_connection(db);

		return NULL;
	}

	db_connection* db_conn = malloc(sizeof(db_connection));
	db_conn->db = db;

	return db_conn;
}

void cleanup_db_connection(db_connection* db_conn) {
	if (db_conn != NULL) {
		close_sqlite_connection(db_conn->db);
		free(db_conn);
	}
}


int dhcp_lease_allocate_temp_ip(db_connection* db_conn, struct in_addr* yaddr, unsigned char mac[]) {
	UNUSED(db_conn);
	UNUSED(mac);

	parse_inaddr("192.168.0.50", yaddr);
	return DHCP_LEASE_OK;
}

int dhcp_lease_allocate_ip(db_connection* db_conn, struct in_addr yaddr, unsigned char mac[]) {
	return dhcp_lease_allocate_temp_ip(db_conn, &yaddr, mac);
}

int dhcp_lease_is_available_ip(db_connection* db_conn, struct in_addr yaddr, unsigned char mac[]) {
	UNUSED(db_conn);
	UNUSED(yaddr);
	UNUSED(mac);

	return DHCP_LEASE_IP_IN_USE;
}

int dhcp_lease_release_ip(struct db_connection* db_conn, struct in_addr yaddr, unsigned char mac[]) {
	UNUSED(db_conn);
	UNUSED(yaddr);
	UNUSED(mac);

	return DHCP_LEASE_OK;
}

