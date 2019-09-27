#ifndef __H_DHCP_STORE
#define __H_DHCP_STORE

#include <arpa/inet.h>


struct db_connection;

struct db_connection* init_db_connection();
void cleanup_db_connection(struct db_connection* db_conn);


int dhcp_lease_allocate_temp_ip(struct db_connection* db_conn, struct in_addr* yaddr, unsigned char mac[]);
int dhcp_lease_allocate_ip(struct db_connection* db_conn, struct in_addr yaddr, unsigned char mac[]);

int dhcp_lease_is_available_ip(struct db_connection* db_conn, struct in_addr yaddr, unsigned char mac[]);

int dhcp_lease_release_ip(struct db_connection* db_conn, struct in_addr yaddr, unsigned char mac[]);


#define DHCP_LEASE_OK 0
#define DHCP_LEASE_NO_AVAILABLE 1
#define DHCP_LEASE_IP_IN_USE 2
#define DHCP_LEASE_NOT_FOUND 3
#define DHCP_LEASE_FAILED -1


#endif // __H_DHCP_STORE
